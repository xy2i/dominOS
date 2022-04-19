#ifndef __SYSCALL_HANDLER_H__
#define __SYSCALL_HANDLER_H__

#define NUM_SYSCALLS 29

// Definitions accessible from asm code
int   num_syscalls;
void *syscalls[NUM_SYSCALLS];

/**
 * Handles syscalls: any int 49 will be caught by this handler
 */
void init_syscall_handler(void);

#endif //__SYSCALL_HANDLER_H__
