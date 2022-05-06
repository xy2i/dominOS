#include <stdint.h>
#include "mem.h"
#include "page_allocator.h"
#include "stdio.h"
#include "stddef.h"

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
    int   nb_alloc;
};

//list of free page area
struct free_area area;

void init_alloc()
{
    if (area.nb_alloc == 0) {
        area.map[MAP_SIZE - 1] = (void *)FIRST_ADDRESS;
        // just one block
        *((void **)area.map[MAP_SIZE - 1]) = NULL;
        area.nb_alloc++;
    }
}

void alloc_block()
{
    if (area.nb_alloc == 1) {
        area.map[MAP_SIZE - 1] = (void *)SECOND_ADDRESS;
        // just one block
        *((void **)area.map[MAP_SIZE - 1]) = NULL;
        area.nb_alloc++;
    } else if (area.nb_alloc == 2) {
        area.map[MAP_SIZE - 1] = (void *)THIRD_ADDRESS;
        // just one block
        *((void **)area.map[MAP_SIZE - 1]) = NULL;
        area.nb_alloc++;
    } else {
        panic("can't allocate more pages");
    }
}

uint32_t puiss2(unsigned long size) {
    uint32_t p=0;
    size = size - 1; // allocation start in 0
    while(size) {  // get the largest bit
	p++;
	size >>= 1;
    }
    if ((int) size > (1 << p))
	p++;
    return p;
}


void *alloc_physical_page(int nb_pages) {
    init_alloc();
    assert(nb_pages > 0);
    assert(nb_pages < NB_PAGES_ALLOC);

    //size we want to allocate
    uint32_t size = nb_pages<<SHIFT;

    //index in the map (the index 0 correspond to a size of 4096 that's why we do -12)
    uint32_t index = puiss2(size) - SHIFT;

    //find the first index where there is a block
    uint32_t scan_index = index;
    while(area.map[scan_index] == NULL) {
	scan_index++;

	//reallocate memory if needed
	if(scan_index == MAP_SIZE - 1 && area.map[scan_index] == NULL) {
	    alloc_block();
	    *((void **) area.map[scan_index]) = NULL;
	}
    }

    // Split blocks until to have the needed size
    while(scan_index != index) {
	void *curr_block = area.map[scan_index];
	area.map[scan_index] = *((void **) curr_block);
	scan_index--;
	area.map[scan_index] = curr_block;

	//Buddy calculation
	uint32_t buddy_block = (uint32_t) curr_block ^ (1<<(scan_index+SHIFT));
	*((void **) area.map[scan_index]) = (void *) buddy_block;
	*((void **) buddy_block) = NULL;
    }

    void *ptr = area.map[index];
    area.map[index] = *((void **) ptr);

    return ptr;
}

void free_physical_page(void *physical_page, int nb_pages) {
    //size allocated
    uint32_t size = nb_pages<<SHIFT;
    uint32_t index = puiss2(size) - SHIFT;
    uint32_t buddy_address = ((uint32_t) physical_page) ^ (1<<(index+SHIFT));

    void *scan_ptr = area.map[index];
    void *previous_ptr = NULL;
    while(scan_ptr != NULL) {
	if((uint32_t) scan_ptr == buddy_address) {
	    // Buddy addr is found : regroup + init to do the same on index + 1
	    if(previous_ptr != NULL) {
		*((void **) previous_ptr) = *((void **) scan_ptr);
	    } else {
		area.map[index] = *((void **) scan_ptr);
	    }
	    index++;
	    if(index == MAP_SIZE) {
		break;
	    }
	    physical_page = ((uint32_t) physical_page < buddy_address) ? physical_page : (void *) buddy_address;
	    scan_ptr = area.map[index];
	    buddy_address = ((uint32_t) physical_page) ^ (1<<(index+SHIFT));
	    previous_ptr = NULL;
	    continue;
	}
	previous_ptr = scan_ptr;
	if(scan_ptr != NULL) {
	    scan_ptr = *((void **) scan_ptr);
	}
    }
    void *next_ptr = area.map[index];
    area.map[index] = physical_page;
    *((void **) area.map[index]) = next_ptr;
}
