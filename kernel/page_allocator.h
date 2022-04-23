#ifndef __PF_ALLOCATOR_H__
#define __PF_ALLOCATOR_H__

#include "parameters.h"
/**
 * Alloc the number of page we want
 * @param pnb_pages : the number of pages. if NULL, it is set to 1 automatically
 * @return the address of the pages
 */
void * alloc_physical_page(int nb_pages);

/**
 * Free the block of pages allocate with the buddy algorithm
 * @param physical_page The address of the block
 * @param nb_pages
 */
void   free_physical_page(void *physical_page, int nb_pages);

#endif