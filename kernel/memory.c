#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "page_allocator.h"
#include "processor_structs.h"
#include "interrupts.h"
#include "memory.h"
#include "mem.h"
#include "queue.h"
#include "exit.h"
#include "cpu.h"
#include "isr.h"

#define PAGE_SZ                     4096
#define PAGE_SHIFT                  12
#define PAGE_FAULT_INTERRUPT_NUMBER 14

extern unsigned   pgtab[];
static unsigned * early_pgtab = pgtab;

extern unsigned   pgdir[];
static unsigned * early_pgdir = pgdir;

struct pde {
    uint32_t present            :1;
    uint32_t writeable          :1;
    uint32_t user_access        :1;
    uint32_t write_through      :1;
    uint32_t cache_disabled     :1;
    uint32_t accessed           :1;
    uint32_t ignored_1          :1;
    uint32_t page_size          :1;
    uint32_t ignored_2          :4;
    uint32_t page_table_address :20;
} __attribute__((__packed__));

struct pte {
    uint32_t present               :1;
    uint32_t writeable             :1;
    uint32_t user_access           :1;
    uint32_t write_through         :1;
    uint32_t cache_disabled        :1;
    uint32_t accessed              :1;
    uint32_t dirty                 :1;
    uint32_t page_attribute_table  :1;
    uint32_t global                :1;
    uint32_t ignored               :3;
    uint32_t physical_address      :20;
} __attribute__((__packed__));

struct vm_area {
    uint32_t         start;
    uint32_t         end;
    struct list_link vm_areas;
    uint32_t         writeable       :1;
    uint32_t         user_accessible :1;
};

struct mm {
    struct pde *     page_directory;
    struct list_link vm_areas;
};



/********
 * Misc *
 *******/

static int is_page_size_aligned(uint32_t address)
{
    return !(address & 0xfff);
}

static uint32_t align_page_size(uint32_t address)
{
    return address & 0xfffff000;
}

void switch_virtual_adress_space(struct mm * mm)
{
    uint32_t page_directory;

    if (!mm)
        page_directory = (uint32_t)early_pgdir;
    else
        page_directory = (uint32_t)mm->page_directory;

    __asm__("movl %0, %%cr3" :: "r"(page_directory));
    tss.cr3 = page_directory;
}


/********************
 * Page directories *
 *******************/

static struct pde * alloc_page_directory(void)
{
    struct pde * page_dir = alloc_physical_page();
    memset(page_dir, 0, sizeof(struct pde) * 1024);
    return page_dir;
}

static void free_page_directory(struct pde * page_directory)
{
    memset(page_directory, 0, sizeof(struct pte) * 1024);
    free_physical_page(page_directory);
}

static void fill_pde(struct pde * pde,
                     struct pte * page_table,
                     uint32_t     writeable,
                     uint32_t     user_accessible)
{
    pde->present            = 1;
    pde->writeable          = writeable;
    pde->user_access        = user_accessible;
    pde->write_through      = 0;
    pde->cache_disabled     = 0;
    pde->page_size          = 0;
    pde->accessed           = 0;
    pde->page_table_address = (uint32_t)page_table >> PAGE_SHIFT;
}

static void zero_out_pde(struct pde * pde)
{
    memset(pde, 0, sizeof(struct pde));
}

static struct pte * pde_page_table(struct pde pde) {
    return (struct pte *)(pde.page_table_address << PAGE_SHIFT);
};

static int pde_empty(struct pde pde)
{
    return !pde.present            &&
           !pde.writeable          &&
           !pde.user_access        &&
           !pde.write_through      &&
           !pde.cache_disabled     &&
           !pde.accessed           &&
           !pde.ignored_1          &&
           !pde.page_size          &&
           !pde.ignored_2          &&
           !pde.accessed           &&
           !pde.page_table_address;
}

uint32_t pde_index(uint32_t address)
{
    return address >> 22;
}



/***************
 * Page tables *
 **************/

static struct pte * alloc_page_table(void)
{
    struct pte * page_table = alloc_physical_page();
    memset(page_table, 0, sizeof(struct pte) * 1024);
    return page_table;
}

static void free_page_table(struct pte * page_table)
{
    memset(page_table, 0, sizeof(struct pte) * 1024);
    free_physical_page(page_table);
}

static void fill_pte(struct pte * pte,
                     uint32_t     physical_address,
                     uint32_t     writeable,
                     uint32_t     user_accessible)
{
    pte->present              = 1;
    pte->writeable            = writeable;
    pte->user_access          = user_accessible;
    pte->write_through        = 0;
    pte->cache_disabled       = 0;
    pte->accessed             = 0;
    pte->dirty                = 0;
    pte->page_attribute_table = 0;
    pte->global               = 0;
    pte->physical_address     = physical_address >> PAGE_SHIFT;
}

static void zero_out_pte(struct pte * pte)
{
    memset(pte, 0, sizeof(struct pte));
}

static uint32_t pte_physical_adress(struct pte pte) {
    return pte.physical_address << PAGE_SHIFT;
};

static int pte_empty(struct pte pte)
{
    return !pte.present              &&
           !pte.writeable            &&
           !pte.user_access          &&
           !pte.write_through        &&
           !pte.cache_disabled       &&
           !pte.accessed             &&
           !pte.dirty                &&
           !pte.page_attribute_table &&
           !pte.global               &&
           !pte.ignored              &&
           !pte.physical_address;
}

uint32_t pte_index(uint32_t address)
{
    return (address & 0x3ff000) >> PAGE_SHIFT;
}



/**********************
 * Memory descriptors *
 *********************/

static int check_vm_area_range(uint32_t address)
{
    return pde_index(address) >= 64;
}

struct vm_area * alloc_vm_area(uint32_t start,
                               uint32_t end,
                               uint32_t writeable,
                               uint32_t user_accessible)
{
    struct vm_area * vm_area;

    if (!check_vm_area_range(start))
        return NULL;

    vm_area = mem_alloc(sizeof(struct vm_area));
    if (!vm_area) 
        return NULL;

    vm_area->start           = align_page_size(start);
    vm_area->end             = is_page_size_aligned(end) ? end : align_page_size(end) + PAGE_SZ;
    vm_area->writeable       = writeable;
    vm_area->user_accessible = user_accessible;

    return vm_area;
}

void free_vm_area(struct vm_area * vm_area)
{
    mem_free(vm_area, sizeof(struct vm_area));
}

static int vm_areas_overlap(struct vm_area A, struct vm_area B)
{
    if (A.start == B.start)
        return 1;

    if (A.start < B.start && A.end > B.start)
        return 1;

    if (B.start < A.start && B.end > A.start)
        return 1;

    return 0;
}

struct mm * alloc_mm(void)
{
    struct mm * mm;

    mm = mem_alloc(sizeof(struct mm));
    if (!mm)
        return NULL;

    mm->page_directory = alloc_page_directory();
    INIT_LIST_HEAD(&mm->vm_areas);

    return mm;
}

void free_mm(struct mm * mm)
{
    struct vm_area * cur;
    struct vm_area * tmp;
    unsigned int     i;

    queue_for_each_safe(cur, tmp, &mm->vm_areas, struct vm_area, vm_areas) {
        queue_del(cur, vm_areas);
        unmap_vm_area(mm, cur);
        free_vm_area(cur);
    }

    for (i = 64; i < 1024; i++) {
        if (!pde_empty(mm->page_directory[i])) {
            free_page_table(pde_page_table(mm->page_directory[i]));
            zero_out_pde(&mm->page_directory[i]);
        }
    }

    free_page_directory(mm->page_directory);

    mem_free(mm, sizeof(struct mm));
}

int add_vm_area(struct mm * mm, struct vm_area * vm_area)
{
    struct vm_area * cur;
    queue_for_each(cur, &mm->vm_areas, struct vm_area, vm_areas) {
        if (vm_areas_overlap(*cur, *vm_area))
            return 0;
    }

    queue_add(vm_area, &mm->vm_areas, struct vm_area, vm_areas, start);
    return 1;
}

void map_vm_area(struct mm * mm, struct vm_area * vm_area)
{
    uint32_t     pde_off;
    uint32_t     pte_off;
    uint32_t     virtual_address;
    uint32_t     physical_address;
    struct pde * page_directory;
    struct pte * page_table;

    page_directory = mm->page_directory;

    for (virtual_address = vm_area->start; virtual_address != vm_area->end; virtual_address += PAGE_SZ) {

        pde_off = pde_index(virtual_address);

        if (pde_empty(page_directory[pde_off])) {
            page_table = alloc_page_table();
            fill_pde(&page_directory[pde_off], page_table, 1, 1);
        } else {
            page_table = pde_page_table(page_directory[pde_off]);
        }
        
        pte_off = pte_index(virtual_address);

        printf("vm_area->start = 0x%08x, vm_area->end = 0x%08x\n", vm_area->start, vm_area->end);
        printf("pde_off = 0x%08x, pte_off = 0x%08X\n", pde_off, pte_off);
        printf("page_table = %p\n", page_table);

        if (!pte_empty(page_table[pte_off]))
            BUG();

        physical_address = (uint32_t)alloc_physical_page();
        fill_pte(&page_table[pte_off], physical_address, vm_area->writeable, vm_area->user_accessible);
    }
}

void unmap_vm_area(struct mm * mm, struct vm_area * vm_area)
{
    
    uint32_t     pde_off;
    uint32_t     pte_off;
    uint32_t     virtual_address;
    uint32_t     physical_address;
    struct pde * page_directory;
    struct pte * page_table;

    page_directory = mm->page_directory;

    for (virtual_address = vm_area->start; virtual_address != vm_area->end; virtual_address += PAGE_SZ) {

        pde_off = pde_index(virtual_address);

        if (pde_empty(page_directory[pde_off]))
            BUG();

        page_table = pde_page_table(page_directory[pde_off]);
        pte_off    = pte_index(virtual_address);

        if (pte_empty(page_table[pte_off]))
            BUG();

        physical_address = pte_physical_adress(page_table[pte_off]);
        zero_out_pte(&page_table[pte_off]);
        free_physical_page((void *)physical_address);
    }
}


void do_kernel_mapping(struct mm * mm)
{
    unsigned int i;
    struct pde * page_directory;

    page_directory = mm->page_directory;

    for (i = 0 ; i < 64; i++) {
        fill_pde(&page_directory[i], (struct pte *)&early_pgtab[i * 1024], 1, 0);
    }

    memset(&page_directory[64], 0, sizeof(struct pde) * (1024 - 64));
}


void page_fault_handler(void)
{
    unsigned int ret;
    __asm__("mov %%cr2, %0" : "=r"(ret));
    cli();
    printf("Page fault at: 0x%08X\n", ret);
    //printf("Page fault occured! %s\n", current()->comm);
    __explicit_exit(1);
    sti();
}

void init_page_fault_handler(void)
{
    fill_gate(gate_adress(PAGE_FAULT_INTERRUPT_NUMBER), (uint32_t)page_fault_isr, KERNEL_CS, RING3, TRAP_GATE);
}