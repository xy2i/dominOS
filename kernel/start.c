#include "cpu.h"
#include "task.h"
#include "exit.h"
#include "pid_allocator.h"
#include "swtch.h"
#include "errno.h"
#include "paging.h"
#include "userspace_apps.h"
#include "string.h"

struct startup_context {
    struct cpu_context cpu;
    uint32_t           exit;
    uint32_t           arg;
};

static void set_task_stack(struct task *task_ptr, int (*func_ptr)(void *),
                           void        *arg)
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
    context->cpu.eip = (uint32_t)func_ptr;
    context->exit    = (uint32_t)__unexplicit_exit;
    context->arg     = (uint32_t)arg;

    task_ptr->context = &context->cpu;
}

static struct task *create_task(int prio, const char *name)
{
    /* check priority */
    if (prio > MAX_PRIO || prio < MIN_PRIO) {
        return ERR_PTR(-EINVAL);
    }

    /* Get the corresponding uapp */

    pid_t pid = alloc_pid();
    if (pid < 0) {
        return ERR_PTR(-EAGAIN);
    }

    struct task *task_ptr = alloc_empty_task();
    if (!task_ptr)
        return ERR_PTR(-EAGAIN);

    set_task_starting_up(task_ptr);
    set_task_name(task_ptr, name);
    set_task_pid(task_ptr, pid);
    set_task_priority(task_ptr, prio);
    set_parent_process(task_ptr, current());
    //task_ptr->msg_val = -1;

    // Create virtual address space (page directory), see paging.c
    task_ptr->page_directory = page_directory_create();
    return task_ptr;
}
int start(int (*func_ptr)(void *), unsigned long ssize, int prio,
          const char *name, void *arg)
{
    if (ssize > USTACK_SZ_MAX || ssize == 0)
        return -EINVAL;

    // Get the corresponding user application for this task
    struct uapps *app = get_uapp_by_name(name);
    if (IS_ERR(app)) {
        return -EINVAL;
    }

    struct task *self = create_task(prio, name);
    if (IS_ERR(self))
        return PTR_ERR(self);

    // App code should begin where uapp starts
    int a = (int)func_ptr;
    printf("%d", a);
    set_task_stack(self, app->start, arg);

    set_task_ready(self);
    add_to_global_list(self);

    if (prio >= current()->priority)
        schedule();

    return self->pid;
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
    struct task *idle;

    idle = create_task(MIN_PRIO, "idle");

    if (IS_ERR(idle))
        BUG();

    set_task_stack(idle, __idle_func, NULL);

    add_to_global_list(idle);
    set_idle(idle);
    set_task_running(idle);
}