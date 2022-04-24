#include "cpu.h"
#include "task.h"
#include "exit.h"
#include "pid_allocator.h"
#include "swtch.h"
#include "errno.h"
#include "paging.h"
#include "userspace_apps.h"
#include "string.h"
#include "page_allocator.h"
#include "mem.h"

// User start virtual address, defined in kernel.lds
#define USER_START 0x40000000
// Our choice for the stack: here starts at end of adress space
// and grows downwards
#define USER_STACK_END 0xffffffff

/*
 * struct cpu_context {
uint32_t edi;
uint32_t esi;
uint32_t ebx;
uint32_t ebp;
uint32_t eip;
};
 */
struct startup_context {
    struct cpu_context cpu;
    uint32_t           exit;
    uint32_t           arg;
};

static void set_task_stack(struct task *task_ptr, int (*func_ptr)(void *),
                           void *arg, int ssize, uint8_t *stack_ptr)
{
    /*
        +---------------+<----------- task_ptr->kstack + KSTACK_SZ - 1 <---- 0xffffffff
        |   arg         |
        +---------------+
        |unexplicit_exit|
        +---------------+
        |               |
        |   (func_ptr)  |
        |               |
        |    cpu context|
        |               |
        |               |
        |               |
        +---------------+<-------------  task_ptr->context  <----- ????
        |               |
        |               |
        |               |
        |               |
        +---------------+
    */

    struct startup_context *context =
        (struct startup_context *)(stack_ptr + ssize -
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

int start(const char *name, unsigned long ssize, int prio, void *arg)
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

    // Allocate the application code in kernel managed memory (64-256Mb).
    // To do so, we'll allocate a number of code_pages, and copy the application's
    // code there.
    int code_size = app->end - app->start + 1;
    // Round up, because we need a full page even if we store only some code.
    int nb_code_pages = code_size % PAGE_SIZE + 1; // 2^12

    uint32_t *code_pages = alloc_physical_page(nb_code_pages);
    memcpy(code_pages, app->start, code_size);

    // Map virtual memory for the code.
    map_zone(self->page_directory, USER_START, USER_START + code_size,
             (uint32_t)code_pages, (uint32_t)code_pages + code_size, RW | US);

    self->code_pages    = code_pages;
    self->nb_code_pages = nb_code_pages;

    // Allocate a stack in managed memory.
    // ssize is the number of words to allocate on the stack
    int      real_size      = ssize * 4;
    int      nb_stack_pages = real_size % PAGE_SIZE + 1;
    uint32_t stack_pages    = (uint32_t)alloc_physical_page(nb_stack_pages);

    // Map virtual memory for the stack.
    // The stack grows downwards and starts at the end of memory.
    map_zone(self->page_directory, USER_STACK_END - ssize + 1, USER_STACK_END,
             stack_pages, stack_pages + ssize - 1, RW | US);
    // Set the stack to start at the allocated area.
    set_task_stack(self, (int (*)(void *))USER_START, arg, ssize,
                   (uint8_t *)stack_pages);

    printf("context phys addr: %x\n", (int)self->context);
    // Set the correct virtual adress for the stack.
    // self->context is going to be the next esp.
    self->context = (struct cpu_context *)(USER_STACK_END -
                                           sizeof(struct startup_context) + 1);

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

    set_task_stack(idle, __idle_func, NULL, 4096, mem_alloc(4096));

    add_to_global_list(idle);
    set_idle(idle);
    set_task_running(idle);
}