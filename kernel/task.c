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



/**************
* READY TASKS *
***************/

struct list_link tasks_ready_queue = LIST_HEAD_INIT(tasks_ready_queue);

void set_task_ready(struct task * task_ptr)
{
    task_ptr->state = TASK_READY;
    queue_add(task_ptr, &tasks_ready_queue, struct task, tasks, priority);
}



/**************
* DYING TASKS *
***************/

struct list_link tasks_dying_queue = LIST_HEAD_INIT(tasks_dying_queue);



/*****************
* SLEEPING TASKS *
******************/ 

struct list_link tasks_sleeping_queue = LIST_HEAD_INIT(tasks_sleeping_queue);

void set_task_sleeping(struct task * task_ptr)
{
    task_ptr->state = TASK_SLEEPING;
    queue_add(task_ptr, &tasks_ready_queue, struct task, tasks, wake_time);
}

void try_wakeup_tasks(void)
{
    struct task * task_cur;
    struct task * task_tmp;

    queue_for_each_safe (task_cur, task_tmp, &tasks_sleeping_queue, struct task, tasks) {
        if (task_cur->wake_time <= current_clock()) {
            task_cur->wake_time = 0;
            queue_del(task_cur, tasks);
            set_task_ready(task_cur);
        }
    }
}

void wait_clock(unsigned long clock)
{
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

static void set_task_running(struct task * task_ptr)
{
    task_ptr->state = TASK_RUNNING;
    __running_task = task_ptr;
}



/*************
* SCHEDULING *
**************/

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
    try_wakeup_tasks();

    struct task * old_task = current();
    struct task * new_task = queue_out(&tasks_ready_queue, struct task, tasks);

    if (new_task != NULL /* MIGHT BE CHANGED */ && new_task != old_task) {
        set_task_ready(old_task);
        set_task_running(new_task);
        swtch(&old_task->context, new_task->context);
    } else {
        // Keeps running old_task
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
    // We're allocating 6 extra bytes on the stack to account for
    // our context. See set_task_startup_context().
    task_ptr->stack = mem_alloc(ssize * sizeof(uint32_t) + 6);
    if (task_ptr->stack == NULL) {
	return NULL;
    }
    return task_ptr;
}

static void set_task_startup_context(struct task * task_ptr, int (*func_ptr)(void*), void * arg)
{
    task_ptr->stack[KERNEL_STACK_SIZE - 2] = (uint32_t)func_ptr;
    task_ptr->stack[KERNEL_STACK_SIZE - 1] = (uint32_t)arg;
    task_ptr->context =
	(struct cpu_context *)&task_ptr->stack[KERNEL_STACK_SIZE - 6];
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

/*************
 * IDLE task *
 *************/
static int __attribute__((noreturn)) __idle(void * arg __attribute__((unused))) {
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