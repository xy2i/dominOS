#include "task.h"
#include "queue.h"
#include "errno.h"
#include "cpu.h"
#include "msg.h"

int getprio(int pid)
{
    struct task *task_ptr;

    task_ptr = pid_to_task(pid);
    if (!task_ptr)
        return -ESRCH;

    return task_ptr->priority;
}

static void __update_queue_priority(struct task *task_ptr)
{
    struct list_link *queue_head;

    queue_head = queue_from_state(task_ptr->state, task_ptr->pid);
    if (!queue_head)
        return;

    queue_update(task_ptr, queue_head, struct task, tasks, priority);
}

int chprio(int pid, int priority)
{
    int old_priority;
    struct task *task_ptr;

    task_ptr = pid_to_task(pid);
    if (!task_ptr)
        return -ESRCH;

    if (is_idle(task_ptr))
        return -EINVAL;

    if (is_task_zombie(task_ptr))
        return -ESRCH;

    if (priority > MAX_PRIO || priority < MIN_PRIO)
        return -EINVAL;

    old_priority = task_ptr->priority;
    set_task_priority(task_ptr, priority);
    __update_queue_priority(task_ptr);

    if (is_task_interrupted_msg(task_ptr)) {
        // Need to update the task position in msg queue as well, since its prio changed
        msg_reinsert(task_ptr);
    }

    if (!is_current(task_ptr) && is_task_ready(task_ptr) &&
        task_ptr->priority > current()->priority) {
        printf("case 1 sched");
        schedule();
    }
    if (is_current(task_ptr)) {
        printf("prio of current: %d %d\n", current()->priority, priority);
        struct task *highest_prio_ready =
            queue_top(&tasks_ready_queue, struct task, tasks);
        //        printf("highest prio ready:%d\n", highest_prio_ready->priority);
        //        global_list_debug();
        if (highest_prio_ready->priority > priority) {
            schedule();
        }
    }

    return old_priority;
}