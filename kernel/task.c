#include <cpu.h>
#include <clock.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "mem.h"
#include "cpu.h"
#include "swtch.h"
#include "queue.h"
#include "msg.h"
#include "task.h"
#include "memory.h"
#include "processor_structs.h"

/* States */
#define TASK_STARTUP 0x00
#define TASK_RUNNING 0x01
#define TASK_READY 0x02
#define TASK_SLEEPING 0x03
#define TASK_ZOMBIE 0x04
#define TASK_INTERRUPTED_SEM 0x05
#define TASK_INTERRUPTED_MSG 0x06
#define TASK_INTERRUPTED_IO 0x07
#define TASK_INTERRUPTED_CHILD 0x08

static void debug_print(void);

/*********************
* GENERIC FUNCTIONS *
********************/

inline static int __is_state(struct task *task_ptr, int state)
{
    return task_ptr->state == state;
}

inline static void __set_task_state(struct task *task_ptr, int state,
                                    struct list_link *queue_ptr)
{
    if (task_ptr->state == state)
        return;

    if (!is_current(task_ptr) && !is_task_starting_up(task_ptr) &&
        !is_task_interrupted_msg(task_ptr)) // queue managed by msg.c
        queue_del(task_ptr, tasks);

    task_ptr->state = state;
    queue_add(task_ptr, queue_ptr, struct task, tasks, priority);
}

inline int is_task_starting_up(struct task *task_ptr)
{
    return __is_state(task_ptr, TASK_STARTUP);
}

inline void set_task_starting_up(struct task *task_ptr)
{
    task_ptr->state = TASK_STARTUP;
}

/***************
* RUNNING TASK *
****************/

static struct task *__running_task =
    NULL; // Currently running task. Can be NULL in interrupt context or on startup.

struct task *current(void)
{
    return __running_task;
}

int is_current(struct task *task_ptr)
{
    return __running_task == task_ptr;
}

int is_task_running(struct task *task_ptr)
{
    return __is_state(task_ptr, TASK_RUNNING);
}

void set_task_running(struct task *task_ptr)
{
    if (!is_task_starting_up(task_ptr) && !is_task_running(task_ptr)) {
        queue_del(task_ptr, tasks);
        RESET_LINK(&task_ptr->tasks);
    }

    task_ptr->state = TASK_RUNNING;
    __running_task  = task_ptr;
}

/**************
* READY TASKS *
***************/

struct list_link tasks_ready_queue = LIST_HEAD_INIT(tasks_ready_queue);

int is_task_ready(struct task *task_ptr)
{
    return __is_state(task_ptr, TASK_READY);
}

void set_task_ready(struct task *task_ptr)
{
    __set_task_state(task_ptr, TASK_READY, &tasks_ready_queue);
}

void set_task_ready_or_running(struct task *task_ptr)
{
    task_ptr->state = TASK_READY;
    queue_add(task_ptr, &tasks_ready_queue, struct task, tasks, priority);
    if (task_ptr->priority > current()->priority) {
        schedule();
    }
}

/*****************
* SLEEPING TASKS *
******************/

static struct list_link tasks_sleeping_queue =
    LIST_HEAD_INIT(tasks_sleeping_queue);

int is_task_sleeping(struct task *task_ptr)
{
    return __is_state(task_ptr, TASK_SLEEPING);
}

void set_task_sleeping(struct task *task_ptr)
{
    __set_task_state(task_ptr, TASK_SLEEPING, &tasks_sleeping_queue);
}

static void try_wakeup_tasks(void)
{
    struct task *cur;
    struct task *tmp;
    queue_for_each_safe(cur, tmp, &tasks_sleeping_queue, struct task, tasks)
    {
        if (current_clock() >= cur->wake_time) {
            cur->wake_time = 0;
            set_task_ready(cur);
        }
    }
}

/***************
* ZOMBIE TASKS *
****************/

static struct list_link tasks_zombie_queue = LIST_HEAD_INIT(tasks_zombie_queue);

int is_task_zombie(struct task *task_ptr)
{
    return __is_state(task_ptr, TASK_ZOMBIE);
}

void set_task_zombie(struct task *task_ptr)
{
    __set_task_state(task_ptr, TASK_ZOMBIE, &tasks_zombie_queue);
}

static void reap_zombies(void)
{
    struct task *cur;
    struct task *tmp;
    queue_for_each_safe(cur, tmp, &tasks_zombie_queue, struct task, tasks)
    {
        if (!is_current(cur) &&
            (is_idle(cur->parent) || is_task_zombie(cur->parent))) {
            free_task(cur);
        }
    }
}

/**************************
* INTERRUPTED_CHILD TASKS *
**************************/

static struct list_link tasks_interrupted_child_queue =
    LIST_HEAD_INIT(tasks_interrupted_child_queue);

int is_task_interrupted_child(struct task *task_ptr)
{
    return __is_state(task_ptr, TASK_INTERRUPTED_CHILD);
}

void set_task_interrupted_child(struct task *task_ptr)
{
    __set_task_state(task_ptr, TASK_INTERRUPTED_CHILD,
                     &tasks_interrupted_child_queue);
}

/************************
* INTERRUPTED_MSG TASKS *
************************/

static struct list_link tasks_interrupted_msg_queue =
    LIST_HEAD_INIT(tasks_interrupted_msg_queue);

struct list_link *queue_from_state(int state, int pid)
{
    switch (state) {
    case TASK_READY:
        return &tasks_ready_queue;
    case TASK_ZOMBIE:
        return &tasks_zombie_queue;
    case TASK_SLEEPING:
        return &tasks_sleeping_queue;
    case TASK_INTERRUPTED_CHILD:
        return &tasks_interrupted_child_queue;
    case TASK_INTERRUPTED_MSG:
        return queue_from_msg(pid);
    default:
        return NULL;
    }
}
int is_task_interrupted_msg(struct task *task_ptr)
{
    return __is_state(task_ptr, TASK_INTERRUPTED_MSG);
}

/**
 * Set task to interrupted msg state and schedule.
 */
void set_task_interrupted_msg(struct task *task_ptr)
{
    // Do not use __set_task_state, sicne it thinks it manages its own queue
    task_ptr->state = TASK_INTERRUPTED_MSG;
    schedule();
}

/********************
* Manage all queues *
********************/

/**
 * A list containing all tasks on the system.
 */
static LIST_HEAD(global_task_list);

void add_to_global_list(struct task *self)
{
    queue_add(self, &global_task_list, struct task, global_tasks, pid);
}

void remove_from_global_list(struct task *self)
{
    queue_del(self, global_tasks);
}

void global_list_debug()
{
    struct task *p;
    printf("\npid\tprio\tstate\tnext\tname\n");
    queue_for_each(p, &global_task_list, struct task, global_tasks)
    {
        printf("%d\t%d\t", p->pid, p->priority);

        switch (p->state) {
        case TASK_RUNNING:
            printf("run");
            break;
        case TASK_READY:
            printf("ready");
            break;
        case TASK_SLEEPING:
            printf("sleep");
            break;
        case TASK_ZOMBIE:
            printf("zombie");
            break;
        case TASK_INTERRUPTED_MSG:
            printf("in msg");
            break;
        case TASK_INTERRUPTED_CHILD:
            printf("child");
            break;
        default:
            printf("{%d}", p->state);
        }
        printf("\t");

        if (p->global_tasks.next != 0) {
            printf("%d\t",
                   queue_entry(p->global_tasks.next, struct task, global_tasks)
                       ->pid);
        } else {
            printf("EOL\t");
        }
        printf("\t%s", p->comm);
        printf("\n");
    }
}

/*****************
* Virtual Memory *
*****************/
static struct mm *alloc_task_mm(void)
{
    struct mm *mm;

    mm = alloc_mm();
    if (!mm)
        return NULL;

    do_kernel_mapping(mm);

    return mm;
}

void alloc_user_stack(struct task *task_ptr, uint32_t stack_size)
{
    struct vm_area *vm_area =
        alloc_vm_area(USTACK_START - stack_size, USTACK_START, 1, 1);
    add_vm_area(task_ptr->mm, vm_area);
    map_vm_area(task_ptr->mm, vm_area);
}

/********************
* Memory allocation *
********************/
struct task *alloc_empty_task(void)
{
    struct task *task_ptr;

    task_ptr = mem_alloc(sizeof(struct task));
    if (!task_ptr)
        goto error;

    task_ptr->kstack = mem_alloc(sizeof(*task_ptr->kstack) * KSTACK_SZ);
    if (!task_ptr->kstack)
        goto error_free_task;

    task_ptr->mm = alloc_task_mm();
    if (!task_ptr->mm)
        goto error_free_kstack;

    INIT_LINK(&task_ptr->tasks);
    INIT_LIST_HEAD(&task_ptr->children);

    goto success;

success:
    return task_ptr;

error_free_kstack:
    mem_free(task_ptr->kstack, sizeof(*task_ptr->kstack) * KSTACK_SZ);

error_free_task:
    mem_free(task_ptr, sizeof(struct task));

error:
    return NULL;
}

void free_task(struct task *task_ptr)
{
    if (!IS_LINK_NULL(&task_ptr->tasks))
        queue_del(task_ptr, tasks);

    if (!IS_LINK_NULL(&task_ptr->siblings))
        queue_del(task_ptr, siblings);

    switch_virtual_adress_space(NULL);

    free_mm(task_ptr->mm);
    mem_free(task_ptr->kstack, sizeof(*task_ptr->kstack) * KSTACK_SZ);
    mem_free(task_ptr, sizeof(struct task));
}

/******************
* Setters on task *
******************/

inline void set_task_name(struct task *task_ptr, const char *name)
{
    strncpy(task_ptr->comm, name, COMM_LEN);
}

inline void set_task_priority(struct task *task_ptr, int priority)
{
    task_ptr->priority = priority;
}

inline void set_task_pid(struct task *task_ptr, pid_t pid)
{
    task_ptr->pid = pid;
}

inline void set_task_return_value(struct task *task_ptr, int retval)
{
    task_ptr->retval = retval;
}

inline void set_parent_process(struct task *child, struct task *parent)
{
    child->parent = parent;
    if (parent) {
        queue_add(child, &parent->children, struct task, siblings, priority);
    }
}

/*************
 * IDLE task *
 *************/

static struct task *__idle = NULL;

struct task *idle(void)
{
    return __idle;
}

int is_idle(struct task *task_ptr)
{
    return task_ptr == __idle;
}

void set_idle(struct task *task_ptr)
{
    __idle = task_ptr;
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
bool is_schedule_active = false;

void schedule(void)
{
    if (is_schedule_active) {
        printf("double schedule()!!!\n");
        BUG();
    }
    is_schedule_active = true;

    struct task *new_task;
    struct task *old_task;

    try_wakeup_tasks();
    reap_zombies();

    new_task = queue_top(&tasks_ready_queue, struct task, tasks);
    old_task = current();

    if (!new_task) {
        is_schedule_active = false;
        return;
    }

    printf("old_task->comm: %s, new_task->comm: %s, pdir:%x\n", old_task->comm,
           new_task->comm, (uint32_t)new_task->page_directory);

    if (is_task_running(old_task)) {
        set_task_ready(old_task);
    }
    set_task_running(new_task);

    is_schedule_active = false;
    // Specified in https://ensiwiki.ensimag.fr/index.php?title=Projet_système_:_Aspects_techniques,
    // we must modify the TSS, which holds a saved copy of CR3.
    // This is a structure specific to the CPU, from which the CPU loads some registers
    // after a hardware "task switch" (unrelated to our own).
    tss.cr3 = (int)new_task->page_directory;
    swtch(&old_task->context, new_task->context, new_task->page_directory);
}

/*****************
* Misc functions *
*****************/

struct task *pid_to_task(pid_t pid)
{
    if (current()->pid == pid) {
        return current();
    }

    struct task *cur;
    queue_for_each(cur, &global_task_list, struct task, global_tasks)
    {
        if (cur->pid == pid) {
            return cur;
        }
    }

    return NULL;
}

/*******************
* System interface *
*******************/

void wait_clock(unsigned long clock)
{
    current()->wake_time = current_clock() + clock;
    set_task_sleeping(current());
    schedule();
}

__attribute__((unused)) static void debug_print(void)
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