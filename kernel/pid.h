#ifndef __PID_H__
#define __PID_H__

#define NBPROC 32
#define PID_MAX NBPROC

typedef int pid_t;

pid_t alloc_pid(void);
void free_pid(pid_t pid);

#endif