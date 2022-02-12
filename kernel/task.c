#include <cpu.h>
#include <clock.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "task.h"
#include "../shared/debug.h"
#include "cpu.h"
#include "../shared/string.h"
#include "swtch.h"
#include "queue.h"

extern void *malloc(size_t size);
extern void free(void *bloc);

struct list_link tasks_ready_queue = LIST_HEAD_INIT(tasks_ready_queue);
struct list_link tasks_dying_queue = LIST_HEAD_INIT(tasks_dying_queue);
struct task *running_task = NULL;
struct task *sleeping_tasks = NULL;

void tstA(void);

static void debug_print() {
    struct task *p;
    printf("current: %d\n", running_task->pid);
    printf("ready: [");
    queue_for_each(p, &tasks_ready_queue, struct task, tasks) {
        printf("%d {prio %d}, ", p->pid, p->priority);
    }
    printf("]\n");
    printf("dying: [");
    queue_for_each(p, &tasks_dying_queue, struct task, tasks) {
        printf("%d {prio %d}, ", p->pid, p->priority);
    }
    printf("]\n");
}

void scheduler() {
    cli();
    struct task *saved_running_task = running_task;
    debug_print();
    queue_add(saved_running_task, &tasks_ready_queue, struct task, tasks,
              priority);
    running_task = queue_out(&tasks_ready_queue, struct task, tasks);
    free_dead_tasks();
    swtch(&saved_running_task->context, running_task->context);
    sti();
}

int pid_used(pid_t pid) {
    struct task *current = NULL;

    // Iterate ready tasks
    queue_for_each(current, &tasks_ready_queue, struct task, tasks) {
        if (current->pid == pid) return 1;
    }
    return 0;
}

/*
 * Should be called first by "idle". The pid if idle is then 1 so we change it later
 */
pid_t alloc_pid() {
    pid_t pid_counter = 1;
    while (pid_counter <= PID_MAX) {
        if (!pid_used(pid_counter)) {
            return pid_counter;
        }
        pid_counter++;
    }

    panic("Cannot allocate a pid!");
}

static struct task *alloc_task(char *name, void (*func)(void)) {
    uint32_t pid = alloc_pid();

    struct task *task = malloc(sizeof(struct task));

    task->pid = pid;
    strncpy(task->comm, name, COMM_LEN);
    task->stack = malloc(STACK_SIZE * sizeof(uint32_t));
    task->stack[STACK_SIZE-1] = (uint32_t) exit_task;
    task->stack[STACK_SIZE - 2] = (uint32_t) func;
    task->context = (struct cpu_context *)&task->stack[STACK_SIZE - 6];
    return task;
}

void create_kernel_task(char *name, void (*function)(void)) {
    struct task *task = alloc_task(name, function);
    task->priority = 1;
    task->state = TASK_READY;
    queue_add(task, &tasks_ready_queue, struct task, tasks, priority);
}


void exit_task() {
    // idle can't be killed
    if (running_task->pid == 0) {
        panic("idle process terminated");
    }
    struct task *saved_running_task = running_task;
    saved_running_task->state = TASK_ZOMBIE;
    queue_add(saved_running_task, &tasks_dying_queue, struct task, tasks, priority);
    running_task = queue_out(&tasks_ready_queue, struct task, tasks);
    swtch(&saved_running_task->context, running_task->context);
}

void free_dead_tasks() {
    struct task *current = NULL;
    struct task *prev = NULL;

    queue_for_each(current, &tasks_dying_queue, struct task, tasks) {
        if (prev != NULL) {
            free(prev);
        }
        prev = current;
    }
    if(prev != NULL) {
        free(prev);
    }
    INIT_LIST_HEAD(&tasks_dying_queue);
}

// Set the running task asleep for a specific amount of clock ticks
void sleep(unsigned long clock) {
    running_task->state = TASK_SLEEPING;
    running_task->wake_time = current_clock() + clock;
}

// Check if a task is asleep. If the wake_time as passed, wakes the task up
bool is_asleep(struct task *task){
    if(task->asleep){
        if(current_clock() > task->wake_time) {
            task->asleep = false;
            task->state = TASK_READY;
        }else{
            return true;
        }
    }
    return false;
}

/**
* default task in the kernel
**/
void idle() {
    for(;;) {
        sti();
        hlt();
        cli();
    }
}

void tstA() {
    printf("A");
    //	unsigned long i;
    //    unsigned long j = 0;
    //	while (j < 10) {
    //		printf("A");
    //        sti();
    //		for (i = 0; i < 5000000; i++);
    //        cli();
    //        j++;
    //	}
}

void tstB() {
    unsigned long i;
	while (1) {
        printf("B");
        sti();
        for (i = 0; i < 5000000; i++);
        cli();
    }
}

static void create_idle(void) {
    struct task *idle_task = alloc_task("idle", idle);
    idle_task->pid = 0;
    running_task = idle_task;
}

void init_tasks() {
    cli();
    create_idle();
    create_kernel_task("A", tstA);
    create_kernel_task("B", tstB);
    sti();
}
