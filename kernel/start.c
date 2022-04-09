#include "cpu.h"
#include "task.h"
#include "exit.h"
#include "pid_allocator.h"
#include "swtch.h"
#include "errno.h"

struct startup_context {
    struct cpu_context cpu;
    uint32_t exit;
    uint32_t arg;
};

static void set_task_startup_context(struct task *task_ptr,
                                     int (*func_ptr)(void *), void *arg)
{
    /*
        +---------------+<----------- task_ptr->kstack + KSTACK_SZ - 1
        |   arg         |
        +---------------+
        |unexplicit_exit|
        +---------------+
        |    func_ptr   |
        +---------------+
        |               |
        |               |
        |               |
        |    context    |
        |               |
        |               |
        |               |
        +---------------<-------------  task_ptr->context
        |               |
        |               |
        |               |
        |               |
        +---------------+
    */

    struct startup_context *context =
        (struct startup_context *)(task_ptr->kstack + KSTACK_SZ -
                                   sizeof(struct startup_context));
    context->cpu.edi = 0;
    context->cpu.esi = 0;
    context->cpu.ebx = 0;
    context->cpu.ebp = 0;
    context->cpu.eip = (uint32_t) func_ptr;
    context->exit    = (uint32_t) __unexplicit_exit;
    context->arg     = (uint32_t) arg;

    task_ptr->context = &context->cpu;
}

static struct task * __start_no_sched(int (*func_ptr)(void *),
                                      int prio, const char *name,
                                      void *arg)
{
    /* check priority */
    if (prio > MAX_PRIO || prio < MIN_PRIO) {
        return ERR_PTR(-EINVAL);
    }


    pid_t pid = alloc_pid();
    if (pid < 0) {
        return ERR_PTR(-EAGAIN);
    }

    struct task * task_ptr = alloc_empty_task();
    if (!task_ptr)
        return ERR_PTR(-EAGAIN);

    set_task_starting_up(task_ptr);
    set_task_startup_context(task_ptr, func_ptr, arg);
    set_task_name(task_ptr, name);
    set_task_pid(task_ptr, pid);
    set_task_priority(task_ptr, prio);
    /* set parent process */
    set_parent_process(task_ptr, current());

    return task_ptr;
}

/*
 *
 * Might be needed in future releases.
 *
 *
static inline int start_kernel_task(int (*func_ptr)(void *), int prio, const char *name, void *arg)
{
    struct task * task_ptr;

    task_ptr = __start_no_sched(func_ptr, prio, name, arg);
    if (IS_ERR(task_ptr))
        return PTR_ERR(task_ptr);

    set_task_ready(task_ptr);

    if (prio >= current()->priority)
        schedule();

    return task_ptr->pid;
}
*/

static inline int start_user_task(int (*func_ptr)(void *), unsigned long ssize, int prio, const char *name, void *arg)
{
    struct task * task_ptr;

    if (ssize > USTACK_SZ_MAX || ssize == 0)
        return -EINVAL;

    task_ptr = __start_no_sched(func_ptr, prio, name, arg);
    if (IS_ERR(task_ptr))
        return PTR_ERR(task_ptr);

    alloc_user_stack(task_ptr, ssize);

    //task_ptr->msg_val = -1;

    set_task_ready(task_ptr);
    add_to_global_list(task_ptr);

    if (prio >= current()->priority)
        schedule();

    return task_ptr->pid;
}

int start(int (*func_ptr)(void *), unsigned long ssize, int prio, const char *name, void *arg) {
    return start_user_task(func_ptr, ssize, prio, name, arg);
}

static int __attribute__((noreturn))
__idle_func(void *arg __attribute__((unused)))
{
    for (;;) {
        sti();
        hlt();
        cli();
    }
}

void start_idle(void)
{
    struct task *idle_ptr;

    idle_ptr = __start_no_sched(__idle_func, MIN_PRIO, "idle", NULL);
    if (IS_ERR(idle_ptr))
        BUG();

    add_to_global_list(idle_ptr);
    set_idle(idle_ptr);
    set_task_running(idle_ptr);
}