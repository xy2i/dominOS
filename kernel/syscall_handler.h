#ifndef __SYSCALL_HANDLER_H__
#define __SYSCALL_HANDLER_H__

/**
 * Handles syscalls: any int 49 will be caught by this handler
 */
void init_syscall_handler(void);

#endif //__SYSCALL_HANDLER_H__
