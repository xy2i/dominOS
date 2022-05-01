#include "task.h"
#include "errno.h"
#include "pid_allocator.h"
#include "paging.h"
#include "page_allocator.h"

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

    printf("exiting, got retval: %d\n", retval);
    // Free the code pages.
    free_physical_page(task_ptr->code_pages, task_ptr->nb_code_pages);

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
