#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__
#include <stdint.h>

int                            chprio(int pid, int priority);
void                           clock_settings(unsigned long *quartz, unsigned long *ticks);
uint32_t                       current_clock(void);
void __attribute__((noreturn)) exit(int retval);
int                            getpid(void);
int                            getprio(int pid);
int                            kill(int pid);
int                            pcount(int id, int *count);
int                            pcreate(int count);
int                            pdelete(int id);
int                            preceive(int id,int *msg);
int                            preset(int id);
int                            psend(int id, int msg);
int                            start(int (*func_ptr)(void *), unsigned long ssize, int prio, const char *name, void *arg);
void                           wait_clock(unsigned long clock);
int                            waitpid(int pid, int *retvalp);
void *                         shm_create(const char *key);
void *                         shm_acquire(const char *key);
void                           shm_release(const char *key);

#endif