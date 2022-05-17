#ifndef __START_H__
#define __START_H__

// User start virtual address, defined in kernel.lds
#define USER_START 0x40000000
// Our choice for the stack: here starts at end of adress space
// and grows downwards
#define USER_STACK_END 0xFFF00000

int start(const char *name, unsigned long ssize, int prio, void *arg);

/**
 * Start a kernel task. This task will have no userspace stack.
 * @param pt_func Function to launch
 * @param prio Prio
 * @param name Name of this process
 * @param arg The argument (if any)
 * @return The created task
 */
struct task *start_kernel_task(int (*pt_func)(void *), int prio,
                               const char *name, void *arg);
int start_idle(void);

#endif