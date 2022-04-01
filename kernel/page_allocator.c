#include <stdint.h>
#include "mem.h"
#include "page_allocator.h"
#include "stdio.h"
#include "stddef.h"

typedef uint32_t word_t;
enum { BITS_PER_WORD = sizeof(word_t) * 8 };
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

#define PAGE_SHIFT 12

#ifdef BUDDY_ALLOCATOR

/**
 * Buddy algorithm
 */

//list of free page area
struct free_area area;

void init_alloc() {
    if(area.nb_alloc == 0) {
	area.map[MAP_SIZE-1] = (void *) FIRST_ADDRESS;
	// just one block
	*((void **) area.map[MAP_SIZE-1]) = NULL;
	area.nb_alloc++;
    }
}

void alloc_block() {
    if(area.nb_alloc == 1) {
	area.map[MAP_SIZE-1] = (void *) SECOND_ADDRESS;
	// just one block
	*((void **) area.map[MAP_SIZE-1]) = NULL;
	area.nb_alloc++;
    } else if(area.nb_alloc == 2) {
	area.map[MAP_SIZE-1] = (void *) THIRD_ADDRESS;
	// just one block
	*((void **) area.map[MAP_SIZE-1]) = NULL;
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

#else

/*
 * ---------- Terminology -----------
 *                                   |
 * pfn: Page Frame Number            |
 *                                   |
 * block: A word (A.K.A a uint32_t)  |
 *                                   |
 * -----------------------------------
 */


/* 
 * The following symbols are defined in `kernel.lds`:
 * mem_heap_end = 0x4000000
 * mem_end = 0x10000000
 * 
 * Math 101:
 * x >> 17 == x / PAGE_SIZE / sizeof(word_t) / 8
 */
word_t __framemap[] = {[0 ... (0x4000000 >> 17) - 1] = 0xffffffff,
                        [0x4000000 >> 17 ... (0x10000000 >> 17) - 1] = 0};

/*
 * Marks the page as allocated in the bitmap.
 */
static void set_frame(uint32_t pfn)
{ 
    __framemap[WORD_OFFSET(pfn)] |= (1 << BIT_OFFSET(pfn));
}

/* 
 * Marks the page as free in the bitmap.
 */
static void clear_frame(uint32_t pfn)
{
    __framemap[WORD_OFFSET(pfn)] &= ~(1 << BIT_OFFSET(pfn)); 
}

/*
 * Checks if a page is free.
 */
static int frame_used(uint32_t pfn)
{
    return __framemap[WORD_OFFSET(pfn)] & (1 << BIT_OFFSET(pfn));
}

/*
 * Checks if 32 adjacent page are free.
 * Used to speed up page allocation.
 */
static int block_used(uint32_t block_index)
{
    return __framemap[block_index] == 0xffffffff;
}

/*
 * Allocates a physical page.
 */
void * alloc_physical_page(void)
{
    static uint32_t block_index = 0;
    static uint32_t pfn = 0;

    for (; block_used(block_index); block_index = (block_index + 1) % sizeof(__framemap));

    pfn = block_index << 4;
    for (; frame_used(pfn); pfn++);

    set_frame(pfn);

    return (void *)(pfn << PAGE_SHIFT);
}

/*
 * Free a physical page.
 */
void free_physical_page(void * physical_page)
{
    clear_frame((uint32_t)physical_page >> PAGE_SHIFT);
}

#endif
