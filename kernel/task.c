#include <cpu.h>
#include <clock.h>
#include "task.h"
#include "../shared/debug.h"
#include "cpu.h"
#include "../shared/string.h"
#include "swtch.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct task *table_ready_task = NULL;
struct task *running_task = NULL;
struct task *sleeping_task = NULL;

void scheduler() {
    struct task *saved_runing_task = running_task;
    if(running_task == NULL) {
        queue_add(running_task, &table_ready_task->list, struct task, list, priority);
    }
    running_task = queue_out(&table_ready_task->list, struct task, list);
    swtch(&saved_runing_task->context, &running_task->context);
}

uint32_t available_pid() {
    struct task *current;
    if(table_ready_task == NULL) {
        return 0;
    }
    uint32_t cpt = 1;
    while(cpt < NB_PROC) {
        bool used = false;
        queue_for_each(current, &table_ready_task->list, struct task, list) {
            if(current->pid == cpt) {
                used = true;
            }
        }
        if(!used) {
            return cpt;
        }
        cpt++;
    }
    return NB_PROC;
}

int create_task(char name[COMM_LEN], void (*pf) (void)) {
    uint32_t pid = available_pid();
    if(pid == NB_PROC) {
        return -1;
    }

    struct task *task = malloc(sizeof(struct task));
    task->pid = pid;
    strcpy(task->comm, name);
    task->stack = malloc(STACK_SIZE);
    task->stack[STACK_SIZE - 1] = (int32_t) pf;
    task->state = READY;
    task->priority = 1;
    task->context = malloc(sizeof(struct cpu_context));
    //task->context todo
    task->asleep = false;

    if(table_ready_task == NULL) {
        INIT_LIST_HEAD(&(task->list));
        table_ready_task = task;
        task->state = RUNNING;
    } else {
        queue_add(task, &(table_ready_task->list), struct task, list, priority);
    }

    return pid;
}

// maybe task isn't a parameter but we set asleep running_task
// time is expected in secondes, maybe switch for a nomber of ticks
void sleep(struct task *task, int time) {
    task->asleep = true;
    task->state = SLEEPING;
    task->wake_time = get_time() + (time/get_clock_freq());
}

bool is_asleep(struct task *task){
    if(task->asleep){
        if(get_time() > task->wake_time) {
            task->asleep = false;
            task->state = READY;
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

void tstB()
{
	unsigned long i;
	while (1) {
		printf("B");
        sti();
		for (i = 0; i < 5000000; i++);
        cli();
	}
}

void init_task() {
    char name[COMM_LEN];
    sprintf(name, "idle");
    create_task(name, idle);
    sprintf(name, "A");
    create_task(name, tstA);
    sprintf(name, "B");
    create_task(name, tstB);
}
