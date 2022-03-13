#ifndef __EXIT_H__
#define __EXIT_H__

#include "task.h"

int                            __exit_task(struct task * task_ptr, int retval);
void                           __unexplicit_exit(void);
void                           __explicit_exit(int retval);
void __attribute__((noreturn)) exit(int retval);

#endif