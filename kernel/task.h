#ifndef __TASK_H__
#define __TASK_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "parameters.h"
#include "types.h"
#include "queue.h"

typedef enum { EBX, ESP, EBP, ESI, EDI, CR3, NB_REGS } saved_regs;

struct task {
    struct mm *mm;
    pid_t      pid;
    char       comm[COMM_LEN];
    uint8_t    state;
    // Register saving zone, for context switch
    uint32_t regs[NB_REGS];
    // Kernel-side stack for this process
    uint32_t *kernel_stack;
    uint32_t  ssize;
    // pointer for the global task list
    struct list_link global_tasks;
    // points to next task of same state
    struct list_link tasks;
    struct task     *parent;
    struct list_link children;
    struct list_link siblings;
    int              priority;
    uint32_t         wake_time;
    int              retval;
    // For queues
    int msg_val;
    int wait_start_time;
    // Pages allocated for code and mem (to free)
    uint32_t *code_pages;
    int       nb_code_pages;
};

void set_task_esp(struct task *task_ptr, uint32_t esp);

int  is_task_starting_up(struct task *task_ptr);
void set_task_starting_up(struct task *task_ptr);

struct task *current(void);
int          is_current(struct task *task_ptr);

int  is_task_running(struct task *task_ptr);
void set_task_running(struct task *task_ptr);

extern struct list_link tasks_ready_queue;
int                     is_task_ready(struct task *task_ptr);
void                    set_task_ready(struct task *task_ptr);
void                    set_task_ready_or_running(struct task *task_ptr);

int  is_task_sleeping(struct task *task_ptr);
void set_task_sleeping(struct task *task_ptr);

int  is_task_zombie(struct task *task_ptr);
void set_task_zombie(struct task *task_ptr);

int  is_task_interrupted_child(struct task *task_ptr);
void set_task_interrupted_child(struct task *task_ptr);

int  is_task_interrupted_msg(struct task *task_ptr);
void set_task_interrupted_msg(struct task *task_ptr);

struct list_link *queue_from_state(int state, int pid);
void              add_to_global_list(struct task *self);
void              remove_from_global_list(struct task *self);
void              global_list_debug();

void alloc_user_stack(struct task *task_ptr, uint32_t stack_size);

struct task *alloc_empty_task();
void         free_task(struct task *task_ptr);

void set_task_name(struct task *task_ptr, const char *name);
void set_task_priority(struct task *task_ptr, int priority);
void set_task_pid(struct task *task_ptr, pid_t pid);
void set_task_return_value(struct task *task_ptr, int retval);
void set_parent_process(struct task *child, struct task *parent);

struct task *idle(void);
int          is_idle(struct task *task_ptr);
void         set_idle(struct task *task_ptr);

void preempt_enable(void);
void preempt_disable(void);
bool is_preempt_enabled(void);
void schedule(void);
void schedule_free_old_task(struct task *old_task);

struct task *pid_to_task(pid_t pid);

void wait_clock(unsigned long clock);

#endif