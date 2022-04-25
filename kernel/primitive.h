#ifndef __PRIMITIVE_H__
#define __PRIMITIVE_H__
#include <stdint.h>

/**
 * Change the priority of the task for a given pid.
 * @return -1 if the pid or the priority is invalid, else the old prio of this
 * process
 */
int chprio(int pid, int priority);
/**
 * Get the clock settings, setting quartz and ticks.
 */
void clock_settings(unsigned long *quartz, unsigned long *ticks);
/**
 * Syscall: Write on the terminal.
 * @param str The address to write.
 * @param size Number of bytes to write.
 * @return 0
 */
int cons_write(const char *str, long size);
/**
 * Get the current clock in ticks.
 */
uint32_t current_clock(void);
/**
 * Exit the process normally. Does not return to caller.
 * When the parent calls waitpid(), the retval is passed to the parent.
 * @param retval The return value for this process.
 */
void __attribute__((noreturn)) exit(int retval);
/**
 * Get the pid of this process.
 */
int getpid(void);
/**
 * Get the prio of a process.
 * @return ESRCH if the pid is invalid, else the prio
 */
int getprio(int pid);
/**
 * Terminates the process identified by pid.
 * @return ESRCH if the pid is invalid, EINVAL if the process cannot be killed
 * (kernel process), else the retval of the process
 */
int kill(int pid);
/**
 * Read the amount of data and processes waiting on the given queue.
 * If count is not NULL, place either a negative value equal to the number
 * of processes blocked on an empty queue, or a positive value equal to the sum
 * of the amount of messages in the queue and the number of processes blocked
 * on the queue.
 * @return -1 if id is invalid, else 0
 */
int pcount(int id, int *count);
/**
 * Create a new msg queue with capacity count.
 * @return a negative value if there are no queues available or count <= 0, else
 * the id of the created msg queue
 */
int pcreate(int count);
/**
 * Delete a message queue.
 *
 * All processes that were waiting on the queue switch to the ready state.
 *
 * If these processes have called psend/precieve, then
 * they will return a negative value. All messages in the queue are dropped.
 * @return -1 if id is invalid, else 0
 */
int pdelete(int id);
/**
 * Retrive the first message from the given msg queue, and place it in *msg if
 * the message is not NULL.
 *
 * If the queue was full, precieve will fill it again with a message from the
 * oldest in the highest priority processes that are blocked on the queue,
 * if that process exists. This process becomes either ready or running, based
 * on its priority.
 *
 * If the process was waiting and pdelete/preset is called, then return a
 * negative value.
 *
 * If a process is waiting on an empty list and its priority is changed by
 * chprio, then it becomes the youngest process of its new priority.
 * @return -1 if id is invalid or process blocked and precieve/preset called,
 * else 0
 */
int preceive(int id, int *msg);
/**
 * Reset a message queue.
 *
 * If there were processes blocked on this queue, switch them to ready
 * or running state, according to their priority.
 *
 * If these processes have called psend/precieve, then
 * they will return a negative value.
 *
 * All messages in the queue are dropped.
 */
int preset(int id);
/**
 * Send a message to the given msg queue.
 *
 * If the msg queue is empty and some processes are blocked waiting for a message,
 * then the oldest process in the highest prio ones gets the message.
 *
 * If the msg queue is full, the calling process switches to blocked on full msg state,
 * until a slot is freed to put the msg.
 *
 * Otherwise, the msg can be put directly in the queue.
 *
 * If the process was waiting and pdelete/preset is called, then return a
 * negative value.
 *
 * @return -1 if id is invalid or process blocked and precieve/preset called,
 * else 0
 */
int psend(int id, int msg);
/**
 * Creates a new process.
 * @param ssize Stack size guaranteed to the calling process.
 * @param prio Priority of this process. A higher priority will mean
 * this process will be given more CPU time.
 * @param name Name of the user app for this process.
 * Please include a corresponding application in the users/ folder.
 * @param arg An argument of the process.
 * @return The pid of the process, or -1 if either the arguments were
 * incorrect or there is not enough space to allocte a process.
 */
int start(const char *name, unsigned long ssize, int prio, void *arg);
/**
 * Set the process in sleeping state for a number of clock cycles.
 * @param clock The amount of clock cycles to wait.
 */
void wait_clock(unsigned long clock);
/**
 * wait for the child of a task
 * @param pid : the pid of the child we want to wait, or -1 if we want to wait the first released child
 * @param retvalp : the return value of the child
 * @return -1 if there is an error (no child with the current pid), else
 * the pid of the child
 */
int waitpid(int pid, int *retvalp);
/**
 * Creates a shared memory page of size 4Ko.
 * If the page is allocated and mapped, its virtual address
 * is returned.
 * @param key The page is registered to the kernel with this key.
 * @return NULL for any kind of error (key is NULL, page already exists,
 * out of memory), otherwise return the virtual address of this page.
 */
void *shm_create(const char *key);
/**
 * Get a reference to a shared memory page.
 * If the page is available, it is mapped for this process.
 * @param key The key the page is registered under.
 * @return NULL if the page is not available, otherwise return the virtual
 * address of the page mapped for that process.
 */
void *shm_acquire(const char *key);
/**
 * Notify kernel that the current process wants to release the reference
 * of the given page. If a reference was acquired using shm_acquire, the page
 * is unmapped and the previously returned virtual address is no longer valid
 * Otherwise, this call has no effect.
 *
 * If this call releases the last reference to a page, the corresponding
 * physical page is freed.
 * @param key The key the page is registered under.
 */
void shm_release(const char *key);

#endif