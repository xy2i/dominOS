#include <stdint.h>

typedef uint32_t word_t;
enum { BITS_PER_WORD = sizeof(word_t) * 8 };
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

#define PAGE_SHIFT 12

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

    for(; block_used(block_index); block_index = block_index % sizeof(__framemap));

    pfn = block_index << 4;
    for(; frame_used(pfn); pfn++);

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