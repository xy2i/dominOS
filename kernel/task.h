#ifndef __TASK_H__
#define __TASK_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "../shared/queue.h"
#include "pid.h"

typedef signed int pid_t;

#define TASK_RUNNING 0x00
#define TASK_READY 0x01
#define TASK_INTERRUPTED_SEM 0x02
#define TASK_INTERRUPTED_IO 0x03
#define TASK_INTERRUPTED_CHILD 0x04
#define TASK_SLEEPING 0x05
#define TASK_ZOMBIE 0x06

#define COMM_LEN 16
#define KERNEL_STACK_SIZE 512
#define MIN_PRIO 1
#define MAX_PRIO 256

struct task {
    pid_t pid;
    char comm[COMM_LEN];
    uint8_t state;
    struct cpu_context *context;
    uint32_t *stack;
    // User-provided stack size, excluding RESERVED_STACK_SIZE;
    // see task.c define
    uint64_t stack_size;
    struct list_link tasks;
    int priority;
    uint32_t wake_time;
};



/**************
* READY TASKS *
***************/

/**
 * Sets a task in ready state and add it to ready tasks queue.
 */
void set_task_ready(struct task * task_ptr);



/*****************
* SLEEPING TASKS *
******************/

/**
 * Sets a task in sleeping state and add it to sleeping tasks queue.
 */
void set_task_sleeping(struct task * task_ptr);

/**
 * Tries to wakeup sleeping tasks and put them in ready state.
 */
void try_wakeup_tasks(void);

/**
 * Set the process in sleeping state for a number of clock cycles.
 * @param clock The amount of clock cycles to wait.
 */
void wait_clock(unsigned long clock);


/**
 * Wait the current process for a number of clock cycles.
 * @param clock The amount of clock cycles to wait.
 */
void wait_clock(unsigned long clock);



/***************
* RUNNING TASK *
****************/

/**
 * Returns currently running task.
 */
struct task * current(void);



/*************
* SCHEDULING *
**************/

/**
 * Calls the scheduler.
**/
void schedule();

/**
 * Enables preemption.
 **/
void preempt_enable(void);

/**
 * Disables preemption.
 **/
void preempt_disable(void);

/**
 * Checks if preemption is enabled.
 **/
bool is_preempt_enabled(void);

/**********************
 * Process management *
 **********************/

/**
 * Creates a new process.
 * @param pt_func Pointer to the address that the process should begin
 * executing from.
 * @param ssize Stack size guantreed to the calling process.
 * @param prio Priority of this process. A higher priority will mean
 * this process will be given more CPU time.
 * @param name Name of this process.
 * @param arg An argument of the process.
 * @return The pid of the process, or -1 if either the arguments were
 * incorrect or there is not enough space to allocte a process.
 */
int start(int (*pt_func)(void *), unsigned long ssize, int prio,
	  const char *name, void *arg);

/**
 * Get the pid of this process.
 */
int getpid(void);

/**
 * If the passed pid is invalid, return -1.
 * Otherwise, return the priority of this process.
 */
int getprio(int pid);

/**
 * Terminates the process identified by pid.
 * If no process has this id, return -1;
 * If tried to kill a kernel process, return -2.
 * If the given process was blocked in a queue for a system element,
 * it is removed from that queue.
 */
int kill(int pid);

/*************
 * IDLE task *
 *************/
void create_idle_task(void);

#endif
