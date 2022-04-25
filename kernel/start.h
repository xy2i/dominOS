#ifndef __START_H__
#define __START_H__

// User start virtual address, defined in kernel.lds
#define USER_START 0x40000000
// Our choice for the stack: here starts at end of adress space
// and grows downwards
#define USER_STACK_END 0xffffffff

int  start(const char *name, unsigned long ssize, int prio, void *arg);
void start_idle(void);

#endif