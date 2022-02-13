#ifndef __TASK_H__
#define __TASK_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../shared/queue.h"

typedef signed int pid_t;

#define TASK_RUNNING 0x00
#define TASK_READY 0x01
#define TASK_INTERRUPTED_SEM 0x02
#define TASK_INTERRUPTED_IO 0x03
#define TASK_INTERRUPTED_CHILD 0x04
#define TASK_SLEEPING 0x05
#define TASK_ZOMBIE 0x06

#define COMM_LEN 16
#define NB_PROC 32
#define PID_MAX NB_PROC
#define STACK_SIZE 512

struct task {
    pid_t pid;
    char comm[COMM_LEN];
    uint8_t state;
    struct cpu_context *context;
    uint32_t *stack;
    struct list_link tasks;
    int priority;
    bool asleep;
    uint32_t wake_time;
};

/**
 * manage task and launch them
**/
void scheduler();

/**
* create a new kernel task
* @param1: the name of the task
* @param2: the pointer of the function that define the task
**/
void create_kernel_task(char *name, void (*function)(void));

/**
 * kill the current task
**/
void exit_task();

/**
 * init the default tasks
 **/
void init_tasks();

/**
 * free all dead tasks struct
 **/
void free_dead_tasks();

/**
 * Get the process ID of the calling process
 */
pid_t getpid();

#endif
