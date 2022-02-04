#ifndef __TASK_H__
#define __TASK_H__

#include <inttypes.h>

#define RUNNING 0x00
#define READY 0x01
#define INTERRUPTED_SEM 0x02
#define INTERRUPTED_IO 0x03
#define INTERRUPTED_CHILD 0x04
#define SLEEPING 0x05
#define ZOMBIE 0x06

#define COMM_LEN 16

struct task {
    uint32_t pid;
    char comm[COMM_LEN];
    uint8_t state;
    struct cpu_context context;
    void *stack;
};

#endif
