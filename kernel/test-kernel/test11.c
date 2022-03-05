/*******************************************************************************
 * Test 11
 *
 * Mutex avec un semaphore, regle de priorite sur le mutex.
 ******************************************************************************/
#include "test11.h"
#include "task.h"
#include "shm.h"

int proc_mutex(void *arg)
{
    struct test11_shared *shared = NULL;
    int p = -1;
    int msg;

    (void)arg;
    shared = (struct test11_shared *)shm_acquire("test11_shm");
    assert(shared != NULL);
    p = getprio(getpid());
    assert(p > 0);

    switch (p) {
    case 130:
	msg = 2;
	break;
    case 132:
	msg = 3;
	break;
    case 131:
	msg = 4;
	break;
    case 129:
	msg = 5;
	break;
    default:
	msg = 15;
    }
    printf(" %d", msg);
    xwait(&shared->sem);
    printf(" %d", 139 - p);
    assert(!(shared->in_mutex++));
    chprio(getpid(), 16);
    chprio(getpid(), p);
    shared->in_mutex--;
    xsignal(&shared->sem);
    return 0;
}

int test11_main(void *arg)
{
    struct test11_shared *shared = NULL;
    int pid1, pid2, pid3, pid4;

    (void)arg;
    shared = (struct test11_shared *)shm_create("test11_shm");
    assert(shared != NULL);
    assert(getprio(getpid()) == 128);
    xscreate(&shared->sem);
    shared->in_mutex = 0;
    printf("1");

    pid1 = start(proc_mutex, 4000, 130, "proc_mutex", 0);
    pid2 = start(proc_mutex, 4000, 132, "proc_mutex", 0);
    pid3 = start(proc_mutex, 4000, 131, "proc_mutex", 0);
    pid4 = start(proc_mutex, 4000, 129, "proc_mutex", 0);
    assert(pid1 > 0);
    assert(pid2 > 0);
    assert(pid3 > 0);
    assert(pid4 > 0);
    assert(chprio(getpid(), 160) == 128);
    printf(" 6");
    xsignal(&shared->sem);
    assert(waitpid(-1, 0) == pid2);
    assert(waitpid(-1, 0) == pid3);
    assert(waitpid(-1, 0) == pid1);
    assert(waitpid(-1, 0) == pid4);
    assert(waitpid(-1, 0) < 0);
    assert(chprio(getpid(), 128) == 160);
    xsdelete(&shared->sem);
    printf(" 11.\n");

    return 0;
}
