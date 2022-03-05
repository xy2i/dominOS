#include "debugger.h"
#include "cpu.h"
#include "stdio.h"
#include "stdint.h"
#include "cga.h"
#include "mem.h"
#include "clock.h"
#include "task.h"
#include "msg.h"
#include "shm.h"
#include "test-kernel/test0.h"
#include "test-kernel/test1.h"
#include "test-kernel/test2.h"
#include "test-kernel/test3.h"
#include "test-kernel/test4.h"
#include "test-kernel/test5.h"
#include "test-kernel/test6.h"
#include "test-kernel/test7.h"
#include "test-kernel/test8.h"
#include "test-kernel/test9.h"
#include "test-kernel/test10.h"
#include "test-kernel/test11.h"
#include "test-kernel/test12-msg.h"

int proc1(void *arg __attribute__((unused)))
{
    printf("proc1: pid %d, prio %d\n", getpid(), getprio(getpid()));
    assert(start(proc1, 512, MAX_PRIO, "proc1", NULL) == 0);
    printf("%d kill()ing itself\n", getpid());
    assert(kill(123) != 0);
    exit(2);
    assert(kill(2) == 0);
    return 0;
}

int sleep_proc()
{
    wait_clock(2 * CLOCK_FREQ);
    return 0;
}

int proc2(void *arg __attribute__((unused)))
{
    for (;;) {
	printf("Proc2: Creation of a task\n");
	assert(start(sleep_proc, 512, MAX_PRIO, "sleep_proc", NULL) == 0);
	printf("Proc2: Wait until the end of sleep_proc\n");
	waitpid(-1, NULL);
	printf("Proc2: sleep_proc is finished\n");
    }
}

// Kernel start for tests
void kernel_start(void)
{
    preempt_disable();
    printf("\f");

    // Call interrupt handler builders
    init_clock();
    shm_init();
    sti();
    create_idle_task();

    start_test(test11_main, 512, 128, "test", NULL);

    preempt_enable();

    while (1)
	hlt();
}
