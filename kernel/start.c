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

static void set_task_startup_context(struct task *task_ptr, int (*func_ptr)(void *), void *arg)
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

    struct startup_context * context = (struct startup_context *)(task_ptr->kstack + KSTACK_SZ - sizeof(struct startup_context));
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
                                      unsigned long ssize __attribute__((unused)),
                                      int prio, const char *name,
                                      void *arg)
{
    struct task * task_ptr;
    pid_t pid;

    task_ptr = alloc_empty_task();
    if (!task_ptr)
        return ERR_PTR(-EAGAIN);

    set_task_starting_up(task_ptr);
    set_task_startup_context(task_ptr, func_ptr, arg);
    set_task_name(task_ptr, name);
    
    /* alloc pid */
    pid = alloc_pid();
    if (pid < 0) {
        free_task(task_ptr);
        return ERR_PTR(-EAGAIN);
    }

    set_task_pid(task_ptr, pid);

    /* set priority */
    if (prio > MAX_PRIO || prio < MIN_PRIO) {
        free_pid(pid);
        free_task(task_ptr);
        return ERR_PTR(-EINVAL);
    }

    set_task_priority(task_ptr, prio);

    /* Check user stack size : currently required for tests only */
    if (ssize > USTACK_SZ_MAX) {
        free_pid(pid);
        free_task(task_ptr);
        return ERR_PTR(-EINVAL);
    }

    //alloc_user_stack(task_ptr, ssize);


    /* set parent process */
    set_parent_process(task_ptr, current());

    return task_ptr;
}

int start(int (*func_ptr)(void *), unsigned long ssize __attribute__((unused)), int prio, const char *name, void *arg)
{
    struct task * task_ptr;

    task_ptr = __start_no_sched(func_ptr, ssize, prio, name, arg);
    if (IS_ERR(task_ptr))
        return PTR_ERR(task_ptr);

    set_task_ready(task_ptr);

    if (prio >= current()->priority)
        schedule();

    return task_ptr->pid;
}



static int __attribute__((noreturn)) __idle_func(void *arg __attribute__((unused)))
{
    for (;;) {
	    sti();
	    hlt();
	    cli();
    }
}

void start_idle(void)
{
    struct task * idle_ptr;

    idle_ptr = __start_no_sched(__idle_func, 0, MIN_PRIO, "idle", NULL);
    if (IS_ERR(idle_ptr))
        BUG();

    set_idle(idle_ptr);
    set_task_running(idle_ptr);
}