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

extern void *malloc(size_t size);

struct list_link tasks_ready_queue = LIST_HEAD_INIT(tasks_ready_queue);
struct task *running_task = NULL;
struct task *sleeping_tasks = NULL;

void scheduler() {
    struct task *saved_runing_task = running_task;
    queue_add(running_task, &tasks_ready_queue, struct task, tasks, priority);
    running_task = queue_out(&tasks_ready_queue, struct task, tasks);
    swtch(&saved_runing_task->context, &running_task->context);
}

int pid_used(pid_t pid) {
    struct task *current = NULL;

    // Check current task
    if (current != NULL && current->pid == pid) return 1;

    // Iterate ready tasks
    queue_for_each(current, &tasks_ready_queue, struct task, tasks) {
        if (current->pid == pid) return 1;
    }

    return 0;
}

/*
 * Should be called first by "idle".
 */
pid_t alloc_pid() {
    pid_t pid_counter = 0;
    while (pid_counter <= PID_MAX) {
        if (!pid_used(pid_counter)) {
            return pid_counter;
        }
    }

    panic("Cannot allocate a pid!");
}

static struct task *alloc_task(char *name, void (*func)(void)) {
    uint32_t pid = alloc_pid();

    struct task *task = malloc(sizeof(struct task));

    task->pid = pid;
    strncpy(task->comm, name, COMM_LEN);
    task->stack = malloc(STACK_SIZE * sizeof(uint32_t));
    task->context = malloc(sizeof(struct cpu_context));
    task->stack[STACK_SIZE - 6] = (uint32_t) func;
    task->stack[STACK_SIZE - 1] = (uint32_t) &task->stack[STACK_SIZE - 1];

    return task;
}

void create_kernel_task(char *name, void (*function)(void)) {
    struct task *task = alloc_task(name, function);
    task->priority = 1;
    task->state = TASK_READY;
    queue_add(task, &tasks_ready_queue, struct task, tasks, priority);
}

// Set the running task asleep for a specific amount of clock ticks
void sleep(unsigned long clock) {
    running_task->state = TASK_SLEEPING;
    running_task->wake_time = current_clock() + clock;
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

void tstA()
{
	unsigned long i;
	while (1) {
		printf("A");
        sti();
		for (i = 0; i < 5000000; i++);
        cli();
	}
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
    running_task = idle_task;
}

void init_tasks() {
    create_idle();
    create_kernel_task("A", tstA);
    create_kernel_task("B", tstB);
}
