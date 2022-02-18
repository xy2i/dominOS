#include <cpu.h>
#include <clock.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "mem.h"
#include "task.h"
#include "../shared/debug.h"
#include "cpu.h"
#include "../shared/string.h"
#include "swtch.h"
#include "queue.h"
#include "mem.h"

/*
 * Space reserved on each task's stack.
 * This is useful to store pointer to exit, the context of each process
 */
#define RESERVED_STACK_SIZE 7

/**************
* READY TASKS *
***************/

struct list_link tasks_ready_queue = LIST_HEAD_INIT(tasks_ready_queue);

void set_task_ready(struct task *task_ptr)
{
    task_ptr->state = TASK_READY;
    queue_add(task_ptr, &tasks_ready_queue, struct task, tasks, priority);
}

/**************
* DYING TASKS *
***************/

struct list_link tasks_zombie_queue = LIST_HEAD_INIT(tasks_zombie_queue);

void set_task_zombie(struct task *task_ptr)
{
    task_ptr->state = TASK_ZOMBIE;
    queue_add(task_ptr, &tasks_zombie_queue, struct task, tasks,
	      state); // no ordering
}

/*
 * Exit a process. This function is put at the bottom of the stack of each
 * task, so it is run even if the task does not explicitly call exit().
 */
void __exit()
{
    cli(); // No interrupts.
    if (current()->pid == 0) {
	panic("idle process terminated");
    }
    set_task_zombie(current());
    free_pid(getpid());
    schedule();
}

/*
 * Free each zombie stack.
 */
void free_zombie_tasks()
{
    struct task *current = NULL;
    struct task *prev = NULL;

    queue_for_each(current, &tasks_zombie_queue, struct task, tasks)
    {
	if (prev != NULL) {
	    mem_free(prev->stack, (prev->stack_size + RESERVED_STACK_SIZE) *
				      sizeof(uint32_t));
	}
	prev = current;
    }
    if (prev != NULL) {
	mem_free(prev->stack,
		 (prev->stack_size + RESERVED_STACK_SIZE) * sizeof(uint32_t));
    }
    INIT_LIST_HEAD(&tasks_zombie_queue);
}

/*****************
* SLEEPING TASKS *
******************/

struct list_link tasks_sleeping_queue = LIST_HEAD_INIT(tasks_sleeping_queue);

void set_task_sleeping(struct task *task_ptr)
{
    task_ptr->state = TASK_SLEEPING;
    queue_add(task_ptr, &tasks_sleeping_queue, struct task, tasks, wake_time);
}

void try_wakeup_tasks(void)
{
    struct task *task_cur;
    struct task *task_tmp;

    queue_for_each_safe(task_cur, task_tmp, &tasks_sleeping_queue, struct task,
			tasks)
    {
	if (current_clock() >= task_cur->wake_time) {
	    task_cur->wake_time = 0;
	    queue_del(task_cur, tasks);
	    set_task_ready(task_cur);
	}
    }
}

void wait_clock(unsigned long clock)
{
    cli(); // No interrupts.
    current()->wake_time = current_clock() + clock;
    set_task_sleeping(current());
    schedule();
}

/***************
* RUNNING TASK *
****************/

static struct task *__running_task = NULL; // Currently running task. Can be NULL in interrupt context or on startup.

struct task * current(void)
{
    return __running_task;
}

static void set_task_running(struct task *task_ptr)
{
    task_ptr->state = TASK_RUNNING;
    __running_task = task_ptr;
}

/*************
* SCHEDULING *
**************/

/* debug */
__attribute__((unused)) static void debug_print()
{
    struct task *p;
    printf("current: %d\n", current()->pid);
    printf("ready: [");
    queue_for_each(p, &tasks_ready_queue, struct task, tasks)
    {
	assert(p->state == TASK_READY);
	printf("%d {prio %d}, ", p->pid, p->priority);
    }
    printf("]\n");
    printf("dying: [");
    queue_for_each(p, &tasks_zombie_queue, struct task, tasks)
    {
	assert(p->state == TASK_ZOMBIE);
	printf("%d {prio %d}, ", p->pid, p->priority);
    }
    printf("]\n");
    printf("sleeping: [");
    queue_for_each(p, &tasks_sleeping_queue, struct task, tasks)
    {
	assert(p->state == TASK_SLEEPING);
	printf("%d {wake %d}, ", p->pid, p->wake_time);
    }
    printf("]\n");
}

static bool __preempt_enabled = false;

void preempt_enable(void)
{
    __preempt_enabled = true;
}

void preempt_disable(void)
{
    __preempt_enabled = false;
}

bool is_preempt_enabled(void)
{
    return __preempt_enabled;
}

void schedule()
{
    /* We can arrive at this function under these circumstances:
     * 1. Preemption, ie. the clock handler was called, and interrupts the
     * currently running process. In this state, old_task->state == TASK_RUNNING.
     * 2. Via an explicit wait_clock() call. In this state,
     * old_task->state == TASK_SLEEPING.
     * 3. Via an explicit exit() call. In this state,
     * old_task->state == TASK_ZOMBIE.
     * 4. The task has exited from its main function and has now called the exit_task
     * function, at the bottom of the stack, which calls this. In this state,
     * old_task->state == TASK_ZOMBIE.
     * TODO: There's likely a way to refactor this so that we don't call schedule
     *  every time, maybe for the explicit calls,
     *  break it up so we don't do all this special handling here?
     */

    // This function should not be interrupted. The context switch will set the
    // interrupt flags (apparently via using eflags, see https://chamilo.grenoble-inp.fr/courses/ENSIMAG4MMPCSEF/document/processus.pdf),
    // but I don't understand how it works.
    cli();
    //debug_print();

    struct task *old_task = current();
    struct task *new_task = queue_out(&tasks_ready_queue, struct task, tasks);

    if (new_task != NULL /* MIGHT BE CHANGED */ && new_task != old_task) {
	// if the task was in another state, then it was added to another queue
	// by wait_clock(), exit()...
	if (old_task->state == TASK_RUNNING) {
	    set_task_ready(old_task);
	}
	set_task_running(new_task);

	// If the current process is zombie, don't free it yet.
	// We need its stack around to perform the context switch.
	if (old_task->state != TASK_ZOMBIE) {
	    free_zombie_tasks();
	}

	// try_wakeup_tasks updates the state of each woken up task as a side effect.
	try_wakeup_tasks();
	swtch(old_task->context, new_task->context);
    } else {
	// Keeps running old_task
	try_wakeup_tasks();
    }
}

/**********************
 * Process management *
 **********************/

static struct task *alloc_empty_task(int ssize)
{
    struct task *task_ptr = mem_alloc(sizeof(struct task));
    if (task_ptr == NULL) {
	return NULL;
    }

    task_ptr->stack_size = ssize;
    // We're allocating some extra bytes on the stack to account for
    // our context. See set_task_startup_context().
    task_ptr->stack = mem_alloc((task_ptr->stack_size + RESERVED_STACK_SIZE) *
				sizeof(uint32_t));
    if (task_ptr->stack == NULL) {
	return NULL;
    }

    return task_ptr;
}

static void set_task_startup_context(struct task *task_ptr,
				     int (*func_ptr)(void *),
				     __attribute__((unused)) void *arg)
{
    uint32_t stack_size = task_ptr->stack_size + RESERVED_STACK_SIZE;
    task_ptr->context = (struct cpu_context *)&task_ptr->stack[stack_size - 5];
    // esp is item 4 : [edi, esi, ebp, esp, ebx]
    task_ptr->stack[stack_size - 4] =
	(uint32_t)&task_ptr->stack[stack_size - 7];
    task_ptr->stack[stack_size - 7] = (uint32_t)func_ptr;
    task_ptr->stack[stack_size - 6] = (uint32_t)__exit;
}

static void set_task_name(struct task * task_ptr, const char * name)
{
    strncpy(task_ptr->comm, name, COMM_LEN);
}

static void set_task_priority(struct task * task_ptr, int priority)
{
    assert(!(priority < MIN_PRIO || priority > MAX_PRIO));
    task_ptr->priority = priority;
}

int start(int (*pt_func)(void *), unsigned long ssize, int prio,
	  const char *name, void *arg)
{
    if (prio < MIN_PRIO || prio > MAX_PRIO)
	return -1; // invalid prio argument

    struct task *task_ptr = alloc_empty_task(ssize);
    if (task_ptr == NULL)
	return -1; // allocation failure

    task_ptr->pid = alloc_pid();
    set_task_name(task_ptr, name);
    set_task_startup_context(task_ptr, pt_func, arg);
    set_task_priority(task_ptr, prio);
    set_task_ready(task_ptr);
    return 0;
}

int getpid()
{
    return current()->pid;
}

/*
 * Look for a task. If no task was found, return NULL.
 */
struct task *find_task(int pid)
{
    if (current()->pid == pid) {
	return current();
    }

    struct task *p;
    queue_for_each(p, &tasks_ready_queue, struct task, tasks)
    {
	if (p->pid == pid) {
	    return p;
	}
    }
    queue_for_each(p, &tasks_zombie_queue, struct task, tasks)
    {
	if (p->pid == pid) {
	    return p;
	}
    }
    queue_for_each(p, &tasks_sleeping_queue, struct task, tasks)
    {
	if (p->pid == pid) {
	    return p;
	}
    }
    return NULL;
}

int getprio(int pid)
{
    struct task *task_ptr = find_task(pid);
    if (task_ptr == NULL) {
	return -1; // no matching task found
    }
    return task_ptr->priority;
}

int chprio(int pid, int priority)
{
    struct task *task_ptr = find_task(pid);

    if (priority < MIN_PRIO || priority > MAX_PRIO || task_ptr == NULL) {
        return -1;
    } else {
        queue_del(task_ptr, tasks);
        int former_priority = task_ptr->priority;
        task_ptr->priority = priority;

        switch (task_ptr->state) {
            case TASK_READY:
                queue_add(task_ptr, &tasks_ready_queue, struct task, tasks, priority);
                break;
            case TASK_SLEEPING:
                queue_add(task_ptr, &tasks_sleeping_queue, struct task, tasks, priority);
                break;
            case TASK_ZOMBIE:
                queue_add(task_ptr, &tasks_zombie_queue, struct task, tasks, priority);
                break;
        }

        return former_priority;
    }
}

int kill(int pid)
{
    if (pid == 0) {
	return -2; // tried to kill idle
    }

    struct task *task_ptr = find_task(pid);
    if (task_ptr == NULL) {
	return -1; // no matching task found
    }

    // TODO: manage blocked task

    cli();

    // Edge case: if we're always creating and killing our own process
    // in a loop, free_zombie_tasks is never run leading to OOM.
    // Do it here to make sure we don't miss zombies.
    free_zombie_tasks();

    free_pid(pid);
    set_task_zombie(task_ptr);

    // If we're killing ourselves, schedule out, as otherwise we might
    // keep running and run __exit(), which would try to make this process
    // zombie twice.
    if (current()->pid == pid) {
	schedule();
    }
    sti();
    return 0;
}

void exit(int retval)
{
    current()->retval = retval;
    __exit();

    // GCC has an exit() defined. To conform with its signature, we must make
    // this function noreturn. The easiest way to do so is to add an infinite
    // loop here.
    for (;;)
	;
}

/*************
 * IDLE task *
 *************/
static int __attribute__((noreturn)) __idle(void *arg __attribute__((unused)))
{
    for (;;) {
	sti();
	hlt();
	cli();
    }
}

void create_idle_task(void)
{
    struct task *idle_ptr = alloc_empty_task(KERNEL_STACK_SIZE);
    if (idle_ptr == NULL) {
	BUG();
    }
    idle_ptr->pid = alloc_pid();
    set_task_name(idle_ptr, "idle");
    set_task_startup_context(idle_ptr, __idle, NULL);
    set_task_priority(idle_ptr, MIN_PRIO);
    set_task_running(idle_ptr);
}
