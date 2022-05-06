#include "task.h"
#include "errno.h"
#include "paging.h"

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
        schedule();
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
                schedule();
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
    // From test19:
    // When we're in this syscall, we are in privilege level 0 and can write
    // everywhere, thus we must check this pointer.
    if (!is_user_addr((uint32_t *)current()->regs[CR3], (uint32_t)retvalp)) {
        return -1;
    }

    if (pid <= 0)
        return __wait_any_child(retvalp);
    return __wait_specific_child(pid, retvalp);
}