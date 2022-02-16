#include <stdint.h>
#include <stdbool.h>

#include "pid.h"

typedef uint32_t word_t;
enum { BITS_PER_WORD = sizeof(word_t) * 8 };
#define WORD_OFFSET(b) ((b) / BITS_PER_WORD)
#define BIT_OFFSET(b)  ((b) % BITS_PER_WORD)

word_t __pidmap[PID_MAX % 8 == 0 ? PID_MAX / 8 : PID_MAX / 8 + 1] = {0};

static void set_pid(pid_t pid)
{ 
    __pidmap[WORD_OFFSET(pid)] |= (1 << BIT_OFFSET(pid));
}

static void clear_pid(pid_t pid)
{
    __pidmap[WORD_OFFSET(pid)] &= ~(1 << BIT_OFFSET(pid)); 
}

static bool pid_used(pid_t pid)
{
    return __pidmap[WORD_OFFSET(pid)] & (1 << BIT_OFFSET(pid));
}

pid_t alloc_pid(void)
{
    pid_t pid = 0;
    for(; pid_used(pid); pid++);
    set_pid(pid);
    return pid;
}

void free_pid(pid_t pid)
{
    clear_pid(pid);
}