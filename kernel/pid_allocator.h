#ifndef __PID_ALLOCATOR_H__
#define __PID_ALLOCATOR_H__

#include "parameters.h"
#include "types.h"

pid_t alloc_pid(void);
void free_pid(pid_t pid);

#endif