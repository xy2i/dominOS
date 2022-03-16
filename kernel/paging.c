#include <stdint.h>
#include <string.h>
#include "paging.h"
#include "pf_allocator.h"

extern unsigned pgtab[];
static unsigned * early_pgtab = pgtab;


/******
* CR0 *
*******/


void __enable_paging(void)
{
    __asm__("movl %cr0, %eax\n"
            "orl  $0x80000000, %eax\n"
            "movl %eax, %cr0\n");
}

void __disable_paging(void)
{
    __asm__("movl %cr0, %eax\n"
            "andl $0x7fffffff, %eax\n"
            "movl %eax, %cr0\n"
            "xorl %eax, %eax\n"
            "movl %eax, %cr3\n");
}

/******
* CR3 *
******/

void load_page_directory(struct pde * page_dir)
{
    __asm__("movl %0, %%cr3" :: "r"((uint32_t)page_dir));
}

/****************************
* Page dir/table allocation *
****************************/

#include <stdio.h>
struct pde * alloc_page_directory(void)
{
    struct pde * page_dir = alloc_pf();
    printf("page_dir allocated : %p\n", page_dir);
    memset(page_dir, 0, sizeof(struct pde) * 1024);
    return page_dir;
}

struct pte * alloc_page_table(void)
{
    struct pte * page_table = alloc_pf();
    memset(page_table, 0, sizeof(struct pte) * 1024);
    return page_table;
}

/*****************************
* Page dir/table assignement *
*****************************/

static void pgdir_initialize(struct pde * page_dir)
{
    unsigned int i;

    for (i = 0; i < 64; i++) {
        page_dir[i].present            = 1;
        page_dir[i].writeable          = 1;
        page_dir[i].user_access        = 0;
        page_dir[i].write_through      = 0;
        page_dir[i].cache_disabled     = 0;
        page_dir[i].page_size          = 0;
        page_dir[i].accessed           = 0;
        page_dir[i].page_table_address =  (uint32_t)&early_pgtab[i * 1024] >> 12;
    }

    for (i = 64; i < 1024; i++) {
        memset(&page_dir[i], 0, sizeof(struct pde));   
    }
}

/*
static void fill_pde(struct pde * pde,
                     struct pte * page_table_address,
                     uint32_t writeable,
                     uint32_t user_accessible)
{
    pde->present            = 1;
    pde->writeable          = writeable;
    pde->user_access        = user_accessible;
    pde->write_through      = 0;
    pde->cache_disabled     = 0;
    pde->page_size          = 0;
    pde->accessed           = 0;
    pde->page_table_address = page_table_address >> 12;
}

static void fill_pte(struct pte * pte,
                     void * page_frame_address,
                     uint32_t writeable,
                     uint32_t user_accessible)
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
    pte->page_frame_address   = page_frame_address >> 12;
}

*/

struct pde * empty_virtual_adress_space(void)
{
    struct pde * page_dir;
    page_dir = alloc_page_directory();
    pgdir_initialize(page_dir);

    return page_dir;
}

/*******
* Misc *
*******/

/*
static int pde_empty(struct pde pde)
{
    return (uint32_t)pde == 0;
}

static int pte_empty(struct pte pte)
{
    return (uint32_t)pte == 0;
}

uint32_t pde_index(void * address)
{
    return address >> 22;
}

uint32_t pte_index(void * adresss)
{
    return (address & 0x3ff000) >> 12
}

struct pte * page_table_adress(struct pde pde)
{
    return (struct pte *)pde.page_table_address;
}

static int is_page_size_aligned(void * address)
{
    return !(adress & 0xfff);
}




void unmap_virtual_adress(struct pde * page_dir, void * start, void * end)
{
    void * adress;
    start = start & 0xfffff000;

    if (is_page_size_aligned(end))
        end = end & 0xfffff000;
    else
        end = (end & 0xfffff000) + 1
}


int map_virtual_address(struct pde * page_dir, void * start, void * end, int writeable, int user_accessible)
{

    int pde_off;
    int pte_off;
    struct pte * page_table;
    void * page_frame_address;

    start = start & 0xfffff000;
    if (is_page_size_aligned(end))
        end = end & 0xfffff000;
    else
        end = (end & 0xfffff000) + 1

    for (page_frame_address = start; page_frame_address != end; page_frame_address += 4096) {
        pde_off = pde_index(page_frame_address);
        // TODO: Check PDE rights
        if (pde_empty(page_dir[pde_off])) {
            page_table = alloc_page_table();
            fill_pde(&page_dir[pde_off], pte, writeable, user_accessible);
        } else {
            page_table = page_table_adress(page_dir[pde_off]);
        }

        pte_off = pte_index(page_frame_address);
        if (pte_empty(page_table[pte_off])) {
            fill_pte(&page_table[pte_off], page_frame_address, writeable, user_accessible);
        } else {
            // FAIL ...
        }
    }
}

void vm_initialize(void)
{
    struct pde * page_dir;
    page_dir = alloc_page_directory();
    pgdir_initialize(page_dir);
}

struct pte identity_pte[64 * 1024] __attribute__((aligned(4096))) = {0};
struct pde identity_pde[1024] __attribute__((aligned(4096))) = {0};

void test_CR3(void)
{
    unsigned int i;

    for (i = 0 ; i < 1024 * 64; i++) {
        identity_pte[i].present              = 1;
        identity_pte[i].writeable            = 1;
        identity_pte[i].user_access          = 0;
        identity_pte[i].write_through        = 0;
        identity_pte[i].cache_disabled       = 0;
        identity_pte[i].accessed             = 0;
        identity_pte[i].dirty                = 0;
        identity_pte[i].page_attribute_table = 0;
        identity_pte[i].global               = 0;
        identity_pte[i].page_frame_address   = i;
    }

    for (i = 0; i < 64; i++) {
        identity_pde[i].present            = 1;
        identity_pde[i].writeable          = 1;
        identity_pde[i].user_access        = 0;
        identity_pde[i].write_through      = 0;
        identity_pde[i].cache_disabled     = 0;
        identity_pde[i].page_size          = 0;
        identity_pde[i].accessed           = 0;
        identity_pde[i].page_table_address = ((uint32_t)&identity_pte[1024 * i]) >> 12;
    }

    __asm__("movl %0, %%cr3" :: "r"((uint32_t)identity_pde));
}
*/