#ifndef __PAGING_H__
#define __PAGING_H__

#include "stdint.h"

/**
 * Map a zone of virtual adresses to a zone of physical adresses.
 * A zone is a range of memory (start-end).
 * @pre both zones must be the same size
 * @param flags Flags to set on all pages
 */
void map_zone(uint32_t virt_start, uint32_t virt_end, uint32_t phy_start,
              uint32_t phy_end, uint32_t flags);

/** Create a page directory. */
uint32_t *page_directory_create();

/**
 * Free a page directory and any corresponding page tables, if they were allocated
 * in this page directory.
 */
//void page_directory_destroy(uint32_t *pdir);

#endif