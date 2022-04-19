#include "paging.h"
#include "kalloc.h"
#include "debug.h"
#include "string.h"

// The kernel's page directory
struct page_directory *kernel_directory = 0;

// The current page directory;
struct page_directory *current_directory = 0;

// A bitset of frames - used or free.
uint32_t *frames;
uint32_t  nframes;

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a / (8 * 4))
#define OFFSET_FROM_BIT(a) (a % (8 * 4))

// Static function to set a bit in the frames bitset
static void set_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx   = INDEX_FROM_BIT(frame);
    uint32_t off   = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

// Static function to clear a bit in the frames bitset
static void clear_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx   = INDEX_FROM_BIT(frame);
    uint32_t off   = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
}

// Static function to test if a bit is set.
/*
static uint32_t test_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr / 0x1000;
    uint32_t idx   = INDEX_FROM_BIT(frame);
    uint32_t off   = OFFSET_FROM_BIT(frame);
    return (frames[idx] & (0x1 << off));
}
*/

// Static function to find the first free frame.
static uint32_t first_frame()
{
    uint32_t i, j;
    for (i = 0; i < INDEX_FROM_BIT(nframes); i++) {
        if (frames[i] != 0xFFFFFFFF) // nothing free, exit early.
        {
            // at least one bit is free here.
            for (j = 0; j < 32; j++) {
                uint32_t toTest = 0x1 << j;
                if (!(frames[i] & toTest)) {
                    return i * 4 * 8 + j;
                }
            }
        }
    }
    return -1;
}

// Function to allocate a frame.
void alloc_frame(struct page *page, int is_kernel, int is_writeable)
{
    if (page->frame != 0) {
        return;
    } else {
        uint32_t frame_address = first_frame();
        if (frame_address == (uint32_t)-1) {
            panic("no free frames!!!");
        }
        set_frame(frame_address * 0x1000);
        page->present = 1;
        page->rw      = (is_writeable) ? 1 : 0;
        page->user    = (is_kernel) ? 0 : 1;
        page->frame   = frame_address;
    }
}

// Function to deallocate a frame.
void free_frame(struct page *page)
{
    uint32_t frame;
    if (!(frame = page->frame)) {
        return;
    } else {
        clear_frame(frame);
        page->frame = 0x0;
    }
}

extern uint32_t mem_end;

void initialise_paging()
{
    printf("ola cest le paging nest ce pas\n");
    // Prepare to identity map from 0 to 256MB.
    uint32_t identity_map_end_page = 0x10000000;

    nframes = (identity_map_end_page) / 0x1000;
    frames  = (uint32_t *)kalloc(INDEX_FROM_BIT(nframes));
    printf("nous avons alloué de la mémoire n'est ce pas\n");
    //memset(frames, 0, INDEX_FROM_BIT(nframes));

    // Let's make a page directory.
    kernel_directory  = (struct page_directory *)kalloc(sizeof(struct page));
    current_directory = kernel_directory;

    // We need to identity map (phys addr = virt addr) from
    // 0x0 to the end of used memory, so we can access this
    // transparently, as if paging wasn't enabled.
    // NOTE that we use a while loop here deliberately.
    // inside the loop body we actually change placement_addr
    // by calling kmalloc(). A while loop causes this to be
    // computed on-the-fly rather than once at the start.
    printf("%d", nframes);
    for (uint32_t i = 0; i < nframes; i++) {
        //printf("allocating frame %x\n", i);
        // Kernel code is readable but not writeable from userspace.
        alloc_frame(get_page(i * 0x1000, 1, kernel_directory), 0, 0);
    }
    printf("pages allocated");
    // Before we enable paging, we must register our page fault handler.
    //register_interrupt_handler(14, page_fault);

    // Now, enable paging!
    switch_page_directory(kernel_directory);
}

void switch_page_directory(struct page_directory *dir)
{
    current_directory = dir;
    __asm__ volatile("mov %0, %%cr3" ::"r"(&dir->tablesPhysical));
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    __asm__ volatile("mov %0, %%cr0" ::"r"(cr0));
}

struct page *get_page(uint32_t address, int make, struct page_directory *dir)
{
    // Turn the address into an index.
    address /= 0x1000;
    // Find the page table containing this address.
    uint32_t table_idx = address / 1024;
    if (dir->tables[table_idx]) // If this table is already assigned
    {
        return &dir->tables[table_idx]->pages[address % 1024];
    } else if (make) {
        printf("creating table %d\n", table_idx);
        dir->tables[table_idx] =
            (struct page_table *)kalloc(sizeof(struct page_table));
        dir->tablesPhysical[table_idx] = 0x7; // PRESENT, RW, US.
        return &dir->tables[table_idx]->pages[address % 1024];
    } else {
        return 0;
    }
}