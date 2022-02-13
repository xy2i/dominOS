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
#include "mem.h"

struct list_link tasks_ready_queue = LIST_HEAD_INIT(tasks_ready_queue);
struct list_link tasks_dying_queue = LIST_HEAD_INIT(tasks_dying_queue);
// sleeping queue ordered in reverse by wakeup time:
// last prio = process to wakeup next
struct list_link tasks_sleeping_queue = LIST_HEAD_INIT(tasks_sleeping_queue);
struct task *running_task = NULL;

__attribute__((unused)) static void debug_print() {
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
    printf("sleeping: [");
    queue_for_each(p, &tasks_sleeping_queue, struct task, tasks) {
        printf("%d {wake %d}, ", p->pid, p->wake_time);
    }
    printf("]\n");
}

void scheduler() {
    // debug_print();

    struct task *saved_running_task = running_task;

    if (!queue_empty(&tasks_ready_queue)) {
        running_task = queue_out(&tasks_ready_queue, struct task, tasks);
        running_task->state = TASK_RUNNING;
        saved_running_task->state = TASK_READY;
        queue_add(saved_running_task, &tasks_ready_queue, struct task, tasks,
                  priority);
    }

    free_dead_tasks();

    // Wakeup one task per scheduler() call.
    struct task *wakeup =
        queue_bottom(&tasks_sleeping_queue, struct task, tasks);
    if (wakeup != NULL && wakeup->wake_time <= current_clock()) {
        queue_del(wakeup, tasks);
        wakeup->state = TASK_READY;
        queue_add(wakeup, &tasks_ready_queue, struct task, tasks, priority);
    }

    if (running_task != NULL) {
        swtch(&saved_running_task->context, running_task->context);
    }
}

bool pid_used(pid_t pid) {
    struct task *current = NULL;

    // Go through  all processes on the system to see if the PID is used.
    // TODO: this is not very efficient
    if (running_task != NULL) {
        if (running_task->pid == pid) {
            return true;
        }
    }
    if (!queue_empty(&tasks_ready_queue)) {
        queue_for_each(current, &tasks_ready_queue, struct task, tasks) {
            if (current->pid == pid)
                return true;
        }
    }
    if (!queue_empty(&tasks_dying_queue)) {
        queue_for_each(current, &tasks_dying_queue, struct task, tasks) {
            if (current->pid == pid)
                return true;
        }
    }
    if (!queue_empty(&tasks_sleeping_queue)) {
        queue_for_each(current, &tasks_sleeping_queue, struct task, tasks) {
            if (current->pid == pid)
                return true;
        }
    }
    return false;
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

    struct task *task = mem_alloc(sizeof(struct task));

    task->pid = pid;
    strncpy(task->comm, name, COMM_LEN);
    task->stack = mem_alloc(STACK_SIZE * sizeof(uint32_t));
    task->stack[STACK_SIZE - 1] = (uint32_t)exit_task;
    task->stack[STACK_SIZE - 2] = (uint32_t)func;
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
    queue_add(saved_running_task, &tasks_dying_queue, struct task, tasks,
              priority);
    running_task = queue_out(&tasks_ready_queue, struct task, tasks);
    swtch(&saved_running_task->context, running_task->context);
}

void wait_clock(unsigned long clock) {
    cli();
    struct task *saved_running_task = running_task;
    running_task->state = TASK_SLEEPING;
    running_task->wake_time = current_clock() + clock;
    queue_add(running_task, &tasks_sleeping_queue, struct task, tasks,
              wake_time);
    running_task = queue_out(&tasks_ready_queue, struct task, tasks);
    swtch(&saved_running_task->context, running_task->context);
}

void free_dead_tasks() {
    struct task *current = NULL;
    struct task *prev = NULL;

    queue_for_each(current, &tasks_dying_queue, struct task, tasks) {
        if (prev != NULL) {
            mem_free(prev, sizeof(struct task));
        }
        prev = current;
    }
    if (prev != NULL) {
        mem_free(prev, sizeof(struct task));
    }
    INIT_LIST_HEAD(&tasks_dying_queue);
}

/**
 * default task in the kernel
 **/
void idle() {
    for (;;) {
        sti();
        hlt();
        cli();
    }
}

void tstA() {
    printf("A");
    unsigned long i;
    unsigned long j = 0;
    while (j < 2) {
        printf("A");
        sti();
        for (i = 0; i < 5000000; i++)
            ;
        cli();
        j++;
    }
}

void tstB() {
    unsigned long i;
    int j = 0;
    while (j <= 10) {
        printf("B");
        sti();
        for (i = 0; i < 5000000; i++)
            ;
        cli();
        j++;
    }
    printf("exiting now");
}

void proc1(void) {
    for (;;) {
        printf("proc1 %d\n", current_clock());
        wait_clock(2 * CLOCKFREQ);
    }
}
void proc2(void) {
    for (int i = 0; i <= 2; i++) {
        printf("proc2 %d\n", current_clock());
        wait_clock(1 * CLOCKFREQ);
    }
    printf("creating procx");
    create_kernel_task("procx", proc1);
    printf("procx done");
}

static void create_idle(void) {
    struct task *idle_task = alloc_task("idle", idle);
    idle_task->pid = 0;
    running_task = idle_task;
}

void init_tasks() {
    cli();
    create_idle();
    create_kernel_task("proc1", proc1);
    create_kernel_task("proc2", proc2);
    create_kernel_task("tstB", tstB);
    sti();
}

pid_t getpid() { return running_task->pid; }
