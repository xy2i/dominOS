#include "task.h"
#include "errno.h"
#include "pid_allocator.h"
#include "paging.h"
#include "page_allocator.h"
#include "clock.h"
#include "msg.h"

static void unlock_interrupted_child_parent(struct task *parent)
{
    if (is_task_interrupted_child(parent))
        set_task_ready(parent);
}

int __exit_task(struct task *task_ptr, int retval)
{
    if (!task_ptr)
        return -ESRCH;

    if (is_idle(task_ptr))
        return -EINVAL;

    if (is_task_zombie(task_ptr))
        return -ESRCH;

    // Now that the task is zombie, use the task->priority field
    // to indicate when the task died, to be compatible with set_task_zombie
    // which always expect the sorting field to be 'priority'.
    // When we will wake up tasks with waitpid, they will stay in the same order.

    // Why UINT32_MAX? In the queue, things are ordered from highest to lowest
    // and we want the opposite order, lowest (earliest died process) should be
    // first.

    // Why? Because the abstractions are ****
    task_ptr->priority = UINT32_MAX - current_clock();

    // If this process was interrupted in a msg queue, remove it from that queue
    struct list_link *queue = queue_from_msg(task_ptr->pid);
    if (queue != NULL) {
        queue_del(task_ptr, tasks);
    }

    remove_from_global_list(task_ptr);
    free_pid(task_ptr->pid);
    set_task_return_value(task_ptr, retval);
    set_task_zombie(task_ptr);
    unlock_interrupted_child_parent(task_ptr->parent);
    return 0;
}

void __attribute__((noreturn)) exit(int retval)
{
    __exit_task(current(), retval);
    schedule();
    panic("exit() did not exit the process %s!", current()->comm);
}
