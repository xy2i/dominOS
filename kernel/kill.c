#include "task.h"
#include "errno.h"
#include "exit.h"

int kill(int pid)
{
    struct task * task_ptr;

    task_ptr = pid_to_task(pid);
    if (!task_ptr)
        return -ESRCH;

    if (is_idle(task_ptr))
        return -EINVAL;

    return __exit_task(task_ptr, 0);
}