/**
 * Start a task.
 *
 * When starting a task, the user process starts at 1Gb
 * (decided for us by build/kernel.lds) ans the stack starts at 4Gb - 1
 * (our choice) growing downwards. We'll call that space the "address space"
 * of the process.
 *
 * Here's a few things we need to respect for paging & user mode:
 *
 * - The task must stay in its address space at all times. The only way
 * it can jump to kernel code is via a syscall. In particular, when exiting,
 * it must not jump back to kernel code.
 *
 * - Doing a syscall is a privileged operation, since it switches from ring3
 * to ring0. When triggering an interrupt with a privilege level change,
 * the stack must be switched. We manage this with both a kernel and a user
 * stack for each process.
 *
 * - When we boot up, we are put directly in kernel mode, and must
 * do a first switch to user mode.
 *
 * Here's an example of privilege level switches
 * with idle and one task doing getprio(), then exiting:
 *
 *  │          kernel mode │ user mode
 *  │ ─────────────────────┼───────────────────────
 *  │   boot               │
 *  │      │         start_idle(); goto_user_mode();
 *  │      └───────────────┬────────────►idle
 *  │                      │              │
 *  │                clock interrupt      │
 *  │  schedule◄───────────┬──────────────┘
 *  │      │switch(idle,task1)
 *  │      │               │
 *  │      │     clock interrupt return
 *  │      └───────────────┬────────────►task_1
 *  │                      │              │
 *  │          getprio syscall (int 49)   │
 *  │ getprio() ◄──────────┬──────────────┘
 *  │      │               │
 *  │      │    syscall handler return
 *  │      └───────────────┬────────────►task1
 *  │                      │             │
 *  │                      │             │end of task1
 *  │                      │             ▼
 *  │                      │      implicit_exit (mapped in user memory)
 *  │                      │             │
 *  │                exit(0) syscall     │
 *  │  exit(0) ◄───────────┼─────────────┘
 *  │     │                │
 *  │     │next task       │
 *  │     │switch(task1,idle)
 *  │     │                │
 *  │     ▼       syscall handler return
 *  │  schedule ────────────────────────►idle
 *  │
 *  │
 *  ▼ time
 */

#include "task.h"
#include "pid_allocator.h"
#include "errno.h"
#include "paging.h"
#include "userspace_apps.h"
#include "string.h"
#include "page_allocator.h"
#include "start.h"
#include "mem.h"
#include "usermode.h"

/**
 * Space reserved on each task's stack.
 * As specified in https://ensiwiki.ensimag.fr/index.php?title=Projet_syst%C3%A8me_:_sp%C3%A9cification#start_-_D.C3.A9marrage_d.27un_nouveau_processus,
 * each process created by start should have 'ssize' usable bytes.
 * Since we need to store extra info on the stack (exit and arg),
 * we account for it here.
 */
// 2 words (uint32_t) = 8 bytes
// Initial esp is not reserved, it will be overwritten
// after the first context switch
#define EXTRA_STACK_SPACE 8

// Used for idle process
void halt()
{
    for (;;) {
        __asm__ __volatile__("hlt" ::: "memory");
    }
}

/**
 * When a process exits manually, it must still be in user mode.
 * Do a syscall to exit(0) here.
 */
void implicit_exit()
{
    int retval;
    __asm__("mov %%eax, %0" : "=r"(retval));
    __asm__("mov $6, %%eax\n"
            "mov %0, %%ebx\n"
            "int $49\n"
            : "=r"(retval));
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

    self->kernel_stack    = mem_alloc(KSTACK_SZ);
    uint8_t *kernel_stack = (uint8_t *)self->kernel_stack;
    kernel_stack += KSTACK_SZ - 1; // Point to the start of stack
    self->regs[ESP0] = (uint32_t)kernel_stack;

    // Create virtual address space (page directory), see paging.c
    self->regs[CR3] = (uint32_t)page_directory_create();

    // Allocate the application code in kernel managed memory (64-256Mb).
    // To do so, we'll allocate a number of code_pages, and copy the application's
    // code there.
    int code_size = app->end - app->start + 1;
    // Round up, because we need a full page even if we store only some code.
    int nb_code_pages = (code_size / PAGE_SIZE) + 1; // 2^12

    uint32_t *code_pages = alloc_physical_page(nb_code_pages);
    memcpy(code_pages, app->start, code_size);

    // Map virtual memory for the code.
    map_zone((uint32_t *)self->regs[CR3], USER_START, USER_START + code_size,
             (uint32_t)code_pages, (uint32_t)code_pages + code_size, RW | US);

    self->code_pages    = code_pages;
    self->nb_code_pages = nb_code_pages;

    // Allocate a stack in managed memory.
    // ssize is the number of words to allocate on the stack,
    // but reserve extra space for exit and arg (see macro comment)
    int      real_size      = ssize * 4 + EXTRA_STACK_SPACE;
    int      nb_stack_pages = (real_size / PAGE_SIZE) + 1;
    uint32_t stack_pages    = (uint32_t)alloc_physical_page(nb_stack_pages);
    self->stack_pages       = (uint32_t *)stack_pages;
    self->nb_stack_pages    = nb_stack_pages;

    // Map virtual memory for the stack.
    // The stack grows downwards and starts at the end of memory.
    map_zone((uint32_t *)self->regs[CR3], USER_STACK_END - real_size + 1,
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

    // Map the __unexplicit_exit function somewhere in a user-readable page,
    // so that after a return from main(), we can exit the kernel properly.
    uint32_t implicit_exit_page      = (uint32_t)alloc_physical_page(1);
    uint32_t implicit_exit_virt_addr = USER_START - PAGE_SIZE;
    map_zone((uint32_t *)self->regs[CR3], implicit_exit_virt_addr,
             implicit_exit_virt_addr + PAGE_SIZE - 1, implicit_exit_page,
             implicit_exit_page + PAGE_SIZE - 1, RW | US);
    memcpy((void *)implicit_exit_page, (void *)implicit_exit, PAGE_SIZE);

    self->exit_pages    = (uint32_t *)implicit_exit_page;
    self->nb_exit_pages = 1;

    // Put values needed to the process on the stack. Stack layout:
    /*
        +---------------+
        |   arg         |
        +---------------+
        |implicit_exit  |
        +---------------+
        |    USER_START |<------------ task start
        |---------------|
        |               |
        |               |
        +---------------+
    */
    uint32_t  size_in_words      = real_size / 4;
    uint32_t *stack_end          = (uint32_t *)(stack_pages);
    stack_end[size_in_words - 1] = (uint32_t)arg;
    stack_end[size_in_words - 2] = (uint32_t)implicit_exit_virt_addr;
    stack_end[size_in_words - 3] = USER_START;

    // Modify esp to point to user start.
    // 3 words on the stack -> point to last one
    self->regs[ESP] = (USER_STACK_END + 1) - (3 * 4);

    printf("cr3 value: %x\n ", self->regs[CR3]);

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
void swtch(uint32_t *old_regs, uint32_t *new_regs);
void start_idle(void)
{
    struct task *idle = start_task("idle", 0x1000 / 4, MIN_PRIO, NULL);
    if (IS_ERR(idle))
        panic("failed to create idle, got retval %d!!", (int)idle);

    add_to_global_list(idle);
    set_idle(idle);
    set_task_running(idle);

    __asm__("movl %0, %%cr3" ::"r"(idle->regs[CR3]));
    goto_user_mode();
}