#include <stdint.h>

typedef uint32_t word_t;
enum { BITS_PER_WORD = sizeof(word_t) * 8 };
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

#define FRAMEMAP_SZ (1 << 15) // 4GO / pagesize / sizeof(word_t) / 8 == 2^32 / 2^12 / 2^2 / 2^3

word_t __framemap[FRAMEMAP_SZ] = {[0 ... 2047] = 0xffffffff,
                                  [2048 ... FRAMEMAP_SZ - 1] = 0};


static void set_frame(uint32_t frame_id)
{ 
    __framemap[WORD_OFFSET(frame_id)] |= (1 << BIT_OFFSET(frame_id));
}

static void clear_frame(uint32_t frame_id)
{
    __framemap[WORD_OFFSET(frame_id)] &= ~(1 << BIT_OFFSET(frame_id)); 
}

static int frame_used(uint32_t frame_id)
{
    return __framemap[WORD_OFFSET(frame_id)] & (1 << BIT_OFFSET(frame_id));
}

static int block_used(uint32_t block_index)
{
    return __framemap[block_index] == 0xffffffff;
}


void * alloc_pf(void)
{
    static uint32_t block_index = 0;
    static uint32_t frame_id = 0;

    for(; block_used(block_index); block_index++);

    frame_id = block_index << 4;
    for(; frame_used(frame_id); frame_id++);

    set_frame(frame_id);

    return (void *)(frame_id << 12);
}

void free_pf(void * frame)
{
    clear_frame((uint32_t)frame >> 12);
}