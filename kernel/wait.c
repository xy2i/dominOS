#include "task.h"
#include "errno.h"

static pid_t __wait_any_child(int *retvalp)
{
    struct task *child;
    struct task *tmp;
    pid_t child_pid;

    while (!queue_empty(&current()->children)) {
        queue_for_each_safe(child, tmp, &current()->children, struct task,
                            siblings)
        {
            if (is_task_zombie(child)) {
                if (retvalp)
                    *retvalp = child->retval;

                child_pid = child->pid;
                free_task(child);
                return child_pid;
            }
        }
        set_task_interrupted_child(current());
        schedule_no_ready();
    }

    return -ECHILD;
}

static pid_t __wait_specific_child(pid_t pid, int *retvalp)
{
    struct task *child;
    struct task *tmp;
    pid_t child_pid;

    if (is_idle(pid_to_task(pid)))
        return -EINVAL;

    queue_for_each_safe(child, tmp, &current()->children, struct task, siblings)
    {
        if (child->pid == pid) {
            while (!is_task_zombie(child)) {
                set_task_interrupted_child(current());
                schedule_no_ready();
            }
            if (retvalp)
                *retvalp = child->retval;

            child_pid = child->pid;
            free_task(child);
            return child_pid;
        }
    }

    return -ECHILD;
}

int waitpid(int pid, int *retvalp)
{
    if (pid > 0)
        return __wait_any_child(retvalp);
    return __wait_specific_child(pid, retvalp);
}