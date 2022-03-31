#ifndef __START_H__
#define __START_H__

int  start(int (*func_ptr)(void *), unsigned long ssize, int prio, const char *name, void *arg);
void start_idle(void);

#endif