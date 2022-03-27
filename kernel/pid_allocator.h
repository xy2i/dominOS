#ifndef __PID_ALLOCATOR_H__
#define __PID_ALLOCATOR_H__

#include "parameters.h"
#include "types.h"

/**
 * Allocates a pid.
 * @return -1 if there is no free pid left, the pid otherwise
 */
pid_t alloc_pid(void);
/**
 * Free a pid: it can be used by another process.
 */
void free_pid(pid_t pid);

#endif