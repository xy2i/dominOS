#ifndef __TASK_H__
#define __TASK_H__

#include <stddef.h>
#include <stdint.h>
#include "../shared/queue.h"

extern void *malloc(size_t size);

#define RUNNING 0x00
#define READY 0x01
#define INTERRUPTED_SEM 0x02
#define INTERRUPTED_IO 0x03
#define INTERRUPTED_CHILD 0x04
#define SLEEPING 0x05
#define ZOMBIE 0x06

#define COMM_LEN 16
#define NB_PROC 32
#define STACK_SIZE 512

struct cpu_context {};

struct task {
    uint32_t pid;
    char comm[COMM_LEN];
    uint8_t state;
    struct cpu_context context;
    int32_t *stack;
    link list;
    int priority;
};

/**
 * manage task and launch them
**/
void scheduler();

/**
* return the first available pid to create a new task
**/
uint32_t available_pid();

/**
* create a new task
* @param1: the name of the task
* @param2: the pointer of the function that define the task
* @return: the pid of the task or -1 if the creation didn't work
**/
int create_task(char name[COMM_LEN], void (*pf) (void));

/**
* init the default task
**/
void init_task();

#endif
