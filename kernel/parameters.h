#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#define CLOCK_FREQUENCY      50

#define MIN_PRIO             1
#define MAX_PRIO             256

#define COMM_LEN             16
#define KSTACK_SZ            2048
#define USTACK_SZ_MAX        8192
//#define USTACK_START         0xffffffff
#define USTACK_START         0xfffff000

#define NBPROC               32
#define PID_MAX              NBPROC
#define PID_MIN              0       // SHOULD NOT BE CHANGED
#define BUDDY_ALLOCATOR

#endif