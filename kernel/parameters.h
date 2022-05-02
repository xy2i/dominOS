#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#define CLOCK_FREQUENCY 50

#define MIN_PRIO 1
#define MAX_PRIO 256

#define COMM_LEN 16
// Size of kernel stack in words
#define KSTACK_SZ 1024
#define USTACK_SZ_MAX 8192

#define NBPROC 32
#define PID_MAX NBPROC - 1
#define PID_MIN 0 // SHOULD NOT BE CHANGED
#define BUDDY_ALLOCATOR

#endif