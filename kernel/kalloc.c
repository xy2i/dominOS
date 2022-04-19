/**
 * Aligned memory allocator.
 * Allows to allocate memory at aligned adresses, mostly for page directories.
 * Inspired from http://www.jamesmolloy.co.uk/tutorial_html/6.-Paging.html
 */

#include "kalloc.h"
#include "stdint.h"
#include "debug.h"

/*
 * These symbols are defined in kernel.lds, and define respectively where the heap ends
 * (as in, our page allocator starts) and when our kernel memory ends.
 * We will allocate blocks within this memory.
 */
/* End of kernel memory heap: 64MB */
#define MEM_BEG 0x4000000
/* End of managed memory: 256MB */
#define MEM_END 0x10000000

uint32_t placement_addr = MEM_BEG; // = mem_heap_end

static int is_not_aligned(uint32_t address)
{
    return (address & 0xfff);
}

// Align the address to 4KB.
static uint32_t align_address(uint32_t address)
{
    address &= 0xFFFFF000; // 0xFFFFF000: inverse of 4096
    address += 0x1000; // 0x1000 = 4KB
    return address;
}

uint32_t kalloc(uint32_t sz)
{
    if (is_not_aligned(placement_addr))
        placement_addr = align_address(placement_addr);

    uint32_t tmp = placement_addr;
    placement_addr += sz;
    if (placement_addr > (MEM_END - 1)) {
        panic("\nout of kernel memory for page allocator!!!");
    }
    return tmp;
}