#include "task.h"
#include "errno.h"
#include "exit.h"

int kill(int pid)
{
    struct task *task_ptr;

    task_ptr = pid_to_task(pid);
    if (!task_ptr)
        return -ESRCH;

    if (is_idle(task_ptr))
        return -EINVAL;

    int ret = __exit_task(task_ptr, 0);

    // If we're killing ourselves, schedule out, as otherwise we might
    // keep running and run exit, which would try to make this process
    // zombie twice.
    if (is_current(task_ptr)) {
        schedule_no_ready();
    }
    return ret;
}