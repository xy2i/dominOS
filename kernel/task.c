#include "task.h"
#include "../shared/debug.h"
#include "cpu.h"
#include "../shared/string.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

struct task *table_ready_task = NULL;
struct task *running_task = NULL;
struct task *sleeping_task = NULL;

void scheduler() {
    queue_add(running_task, &(table_ready_task->list), struct task, list, priority);
    running_task = queue_out(&table_ready_task->list, struct task, list);
    //context_switch
}

uint32_t available_pid() {
    uint32_t cpt = 0;
    struct task *current;

    if(table_ready_task == NULL) {
        return cpt;
    }

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
    return 0;
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
    INIT_LIST_HEAD(&task->list);
    //task->context todo

    if(table_ready_task == NULL) {
        table_ready_task = task;
        running_task = task;
    } else {
        queue_add(task, &(table_ready_task->list), struct task, list, priority);
    }

    return pid;
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
