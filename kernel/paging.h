#ifndef __PAGING_H__
#define __PAGING_H__

#include "stdint.h"

// A page is 4Kb (0x1000)
#define PAGE_SIZE 0x1000
#define PAGE_SIZE_SHIFT 12 // 2^12 = 4Kb

// Flags
// Entry present in page table/directory
#define NONE 0x0
#define PRESENT 0x1
// Read write page
#define RW 0x2
// Page accessible in user mode if true, otherwhise only kernel mode page
#define US 0x4

/**
 * Map a zone of virtual adresses to a zone of physical adresses.
 * A zone is a range of memory (start-end).
 * @pre both zones must be the same size
 * @param pdir The page directory to map on
 * @param flags Flags to set on all pages
 */
void map_zone(uint32_t *pdir, uint32_t virt_start, uint32_t virt_end,
              uint32_t phy_start, uint32_t phy_end, int align_virt_end, uint32_t flags);

/** Create a page directory. */
uint32_t *page_directory_create();

/**
 * Free a page directory and any corresponding page tables, if they were allocated
 * in this page directory.
 */
void page_directory_destroy(uint32_t *dir);

#endif