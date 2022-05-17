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
#include "cpu.h"
#include "usermode.h"
#include "exit.h"

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

struct task *create_task(int prio, const char *name)
{
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
    return self;
}

/**
 * Set startup context, ie the stack of the process.
 * @param stack_start_phy last element on the stack
 */
void set_stack(uint32_t *stack_start_phy, void *arg, uint32_t task_start_virt,
               uint32_t stack_start_virt, uint32_t *regs)
{
    /*
        +---------------+
        |   arg         |<------------ stack_start_phy
        +---------------+
        |implicit_exit  |
        +---------------+
        |    USER_START |<------------ task start
        |---------------|
        |               |
        |               |
        +---------------+
    */
    *stack_start_phy-- = (uint32_t)arg;
    *stack_start_phy-- = (uint32_t)__implicit_exit;
    *stack_start_phy = task_start_virt;

    // Modify esp to point to the stack.
    // 3 words on the stack -> point to last one
    regs[ESP] = (stack_start_virt) - (2 * 4);
}

struct task *start_kernel_task(int (*pt_func)(void *), int prio,
                               const char *name, void *arg)
{
    struct task *self = create_task(prio, name);
    if (IS_ERR(self)) {
        return self;
    }

    uint8_t *kernel_stack = mem_alloc(KSTACK_SZ);
    set_stack((uint32_t *)(kernel_stack + KSTACK_SZ - 4), arg,
              (uint32_t)pt_func, (uint32_t)(kernel_stack + KSTACK_SZ - 4),
              self->regs);

    add_to_global_list(self);
    set_task_ready(self);
    return self;
}

int idle_function(void *arg)
{
    (void)arg;
    for (;;) {
        printf("whats up homies its james carlson");
        sti();
        hlt();
        cli();
    }
}

void swtch(uint32_t *old_regs, uint32_t *new_regs);
int start_idle()
{
    struct task *idle =
        start_kernel_task(idle_function, MIN_PRIO, "idle", NULL);
    if (IS_ERR(idle)) {
        return PTR_ERR(idle);
    }

    set_idle(idle);
    set_task_running(idle);

    uint32_t fake_regs[7] = { 0, 0, 0, 0, 0, 0, 0 };
    swtch(fake_regs, idle->regs);

    return 0;
}

struct task *start_task(const char *name, unsigned long ssize, int prio,
                        void *arg)
{
    (void)prio;
    (void)arg;
    if (ssize > USTACK_SZ_MAX)
        return ERR_PTR(-EINVAL);

    // Get the corresponding user application for this task
    struct uapps *app = get_uapp_by_name(name);
    if (IS_ERR(app)) {
        return ERR_PTR(-EINVAL);
    }
    //
    //    self->kernel_stack = mem_alloc(KSTACK_SZ);
    //    uint8_t *kernel_stack = (uint8_t *)self->kernel_stack;
    //    kernel_stack += KSTACK_SZ - 1; // Point to the start of stack
    //    self->regs[ESP0] = (uint32_t)kernel_stack;
    //
    //    // Create virtual address space (page directory), see paging.c
    //    self->regs[CR3] = (uint32_t)page_directory_create();
    //
    //    // Allocate the application code in kernel managed memory (64-256Mb).
    //    // To do so, we'll allocate a number of code_pages, and copy the application's
    //    // code there.
    //    int code_size = app->end - app->start + 1;
    //    // Round up, because we need a full page even if we store only some code.
    //    int nb_code_pages = (code_size / PAGE_SIZE) + 1; // 2^12
    //
    //    uint32_t *code_pages = alloc_physical_page(nb_code_pages);
    //    memcpy(code_pages, app->start, code_size);
    //
    //    // Map virtual memory for the code.
    //    map_zone((uint32_t *)self->regs[CR3], USER_START, USER_START + code_size,
    //             (uint32_t)code_pages, (uint32_t)code_pages + code_size, RW | US);
    //
    //    self->code_pages    = code_pages;
    //    self->nb_code_pages = nb_code_pages;
    //
    //    // Allocate a stack in managed memory.
    //    // ssize is the number of words to allocate on the stack,
    //    // but reserve extra space for exit and arg (see macro comment)
    //    int      real_size      = ssize * 4 + EXTRA_STACK_SPACE;
    //    int      nb_stack_pages = (real_size / PAGE_SIZE) + 1;
    //    uint32_t stack_pages    = (uint32_t)alloc_physical_page(nb_stack_pages);
    //    self->stack_pages       = (uint32_t *)stack_pages;
    //    self->nb_stack_pages    = nb_stack_pages;
    //
    //    // Map virtual memory for the stack.
    //    // The stack grows downwards and starts at the end of memory.
    //    map_zone((uint32_t *)self->regs[CR3], USER_STACK_END - real_size + 1,
    //             USER_STACK_END, stack_pages, stack_pages + real_size - 1, RW | US);
    //
    //    // We've now allocated enough pages for the stack.
    //    // But if the stack is not a multiple of PAGE_SIZE, then we've allocated more bytes
    //    // than we needed. In the case of the code section above, this isn't an issue,
    //    // because the free space is after the code.
    //    // However, since the stack grows backwards, the free space needs to be
    //    // at the start, not the end!
    //    //
    //    // For example, here's the allocated when we want to create a stack with a size
    //    // of 0x1C00, thus allocating two pages.
    //    // Annotated is where we want the stack to go.
    //    //
    //    //              allocated space (PAGE_SIZE*2)
    //    //◄────────────────────────────────────────────────────────────────────────►
    //    //┌──────────┼─────────────────────────────┼───────────────────────────────┐
    //    //│          │                             │                               │
    //    //│free space│  last page stack            │           rest of stack       │
    //    //│  0x400   │          0xC00              │              PAGE_SIZE        │
    //    //└──────────┴─────────────────────────────┴───────────────────────────────┘
    //    //▲         ▲                                                              stack start
    //    //│         │
    //    //│         │
    //    //│         │
    //    //address returned
    //    //by alloc_physical_page
    //    //          │
    //    //          │
    //    //          address we want
    //    //
    //    // To go from the address returned by the allocator to the adress we want, we must
    //    // fix up the address. To do so, get the last page stack size
    //    // get the rest of the space by - PAGE_SIZE, thus giving us the free space.
    //    // and add it to our pointer.
    //    if (stack_pages & 0xFFFFF000) { // if stack_pages not aligned
    //        stack_pages += PAGE_SIZE - (real_size % PAGE_SIZE);
    //    }
    //
    //    // Put values needed to the process on the stack. Stack layout:
    //
    //    return self;
    return NULL;
}

int start(const char *name, unsigned long ssize, int prio, void *arg)
{
    struct task *task = start_task(name, ssize, prio, arg);
    if (IS_ERR(task)) {
        return PTR_ERR(task);
    }

    task->msg_val = -1;
    set_task_ready(task);
    add_to_global_list(task);

    if (prio >= current()->priority)
        schedule();

    return task->pid;
}
void swtch(uint32_t *old_regs, uint32_t *new_regs);