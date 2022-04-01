#ifndef __PF_ALLOCATOR_H__
#define __PF_ALLOCATOR_H__

#include "parameters.h"

#ifdef BUDDY_ALLOCATOR

/**
* Buddy algorithm to alloc pages
 * Pages have a minimum size of 4096
 * and are located between 0x4000000 and 0x10000000.
 * The size of the given block is 0xc000000, and it's not a power of two.
 * That's why we divide this blocks in three block of 0x4000000 size
 * and we apply the buddy algorithm on these blocks
 * In alloc_pf, you need to allocate a number of pages you want
 * (ie the number of bloc of 4096)
*/

#define FIRST_ADDRESS 0x4000000
#define SECOND_ADDRESS 0x8000000
#define THIRD_ADDRESS 0xc000000

#define NB_PAGES_ALLOC 0x4000
#define MAP_SIZE 15
#define SHIFT 12

struct free_area {
    void *map[MAP_SIZE];
    int nb_alloc;
};

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


#else
void * alloc_physical_page(void);
void   free_physical_page(void * physical_page);
#endif

#endif