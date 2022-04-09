#include <stdint.h>
#include <stdbool.h>

#include "pid_allocator.h"

// word is 32 bits
#define WORD_OFFSET(b) ((b) / 32)
#define BIT_OFFSET(b) ((b) % 32)

/* bitmap */
uint32_t __pidmap[PID_MAX % 8 == 0 ? PID_MAX / 8 : PID_MAX / 8 + 1] = { 0 };

static void set_pid(pid_t pid)
{
    __pidmap[WORD_OFFSET(pid)] |= (1 << BIT_OFFSET(pid));
}

void free_pid(pid_t pid)
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
    for (; pid_used(pid); pid++) {
        if (pid > PID_MAX) {
            return -1;
        }
    }
    set_pid(pid);

    return pid;
}