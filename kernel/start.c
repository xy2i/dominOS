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
#include "usermode.h"
#include "start.h"
#include "startup.h"
#include "processor_structs.h"

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

struct task *start_task(const char *name, unsigned long ssize, int prio,
                        void *arg)
{
    if (ssize > USTACK_SZ_MAX || ssize == 0)
        return ERR_PTR(-EINVAL);

    // Get the corresponding user application for this task
    struct uapps *app = get_uapp_by_name(name);
    if (IS_ERR(app)) {
        return ERR_PTR(-EINVAL);
    }
    /* check priority */
    if (prio > MAX_PRIO || prio < MIN_PRIO) {
        return ERR_PTR(-EINVAL);
    }

    pid_t pid = alloc_pid();
    if (pid < 0) {
        return ERR_PTR(-EAGAIN);
    }

    struct task *self = alloc_empty_task();
    if (!self)
        return ERR_PTR(-EAGAIN);

    set_task_starting_up(self);
    set_task_name(self, name);
    set_task_pid(self, pid);
    set_task_priority(self, prio);
    set_parent_process(self, current());

    // Create virtual address space (page directory), see paging.c
    self->page_directory = page_directory_create();

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
    map_zone(self->page_directory, USER_STACK_END - real_size + 1,
             USER_STACK_END, stack_pages, stack_pages + real_size - 1, RW | US);

    // We've now allocated enough pages for the stack.
    // But if the stack is not a multiple of PAGE_SIZE, then we've allocated more bytes
    // than we needed. In the case of the code section above, this isn't an issue,
    // because the free space is after the code.
    // However, since the stack grows backwards, the free space needs to be
    // at the start, not the end!
    //
    // For example, here's the allocated when we want to create a stack with a size
    // of 0x1C00, thus allocating two pages.
    // Annotated is where we want the stack to go.
    //
    //              allocated space (PAGE_SIZE*2)
    //◄────────────────────────────────────────────────────────────────────────►
    //┌──────────┼─────────────────────────────┼───────────────────────────────┐
    //│          │                             │                               │
    //│free space│  last page stack            │           rest of stack       │
    //│  0x400   │          0xC00              │              PAGE_SIZE        │
    //└──────────┴─────────────────────────────┴───────────────────────────────┘
    //▲         ▲                                                              stack start
    //│         │
    //│         │
    //│         │
    //address returned
    //by alloc_physical_page
    //          │
    //          │
    //          address we want
    //
    // To go from the address returned by the allocator to the adress we want, we must
    // fix up the address. To do so, get the last page stack size
    // get the rest of the space by - PAGE_SIZE, thus giving us the free space.
    // and add it to our pointer.
    if (stack_pages & 0xFFFFF000) { // if stack_pages not aligned
        stack_pages += PAGE_SIZE - (real_size % PAGE_SIZE);
    }

    set_task_stack(self, (int (*)(void *))USER_START, arg, real_size,
                   (uint8_t *)stack_pages);

    // Set the correct virtual adress for the stack.
    // self->context is going to be the next esp.
    self->context = (struct cpu_context *)(USER_STACK_END -
                                           sizeof(struct startup_context) + 1);

    return self;
}

int start(const char *name, unsigned long ssize, int prio, void *arg)
{
    struct task *task = start_task(name, ssize, prio, arg);
    if (IS_ERR(task)) {
        return PTR_ERR(task);
    }

    set_task_ready(task);
    add_to_global_list(task);

    if (prio >= current()->priority)
        schedule();

    return task->pid;
}

void start_idle(void)
{
    struct task *idle = start_task("idle", 0x1000, MIN_PRIO, NULL);
    if (IS_ERR(idle))
        panic("failed to create idle, got retval %d!!", (int)idle);

    add_to_global_list(idle);
    set_idle(idle);
    set_task_running(idle);

    //    // Before jumping to user mode, swap the address space.
    //    __asm__("movl %0, %%cr3" ::"r"(idle->page_directory));
    //    //tss.cr3 = (uint32_t)idle->page_directory;
    //
    //    goto_user_mode((uint32_t)idle->context);
}