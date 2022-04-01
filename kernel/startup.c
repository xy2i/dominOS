#include "cpu.h"
#include "stdio.h"
#include "stdint.h"
#include "clock.h"
#include "start.h"
#include "task.h"
#include "shm.h"
#include "page_allocator.h"
#include "ktests.h"
#include "usermode.h"

#define START_TEST(n)                                                          \
    do {                                                                       \
	printf("Starting test: " #n ".\n");                                    \
	start(test##n, 512, 128, "test" #n, NULL);                             \
	printf("Test " #n " successfull.\n");                                  \
    } while (0)

void kernel_start(void)
{
    printf("\f");

    preempt_disable();
    init_clock();
    init_page_fault_handler();
    init_syscall_handler();
    shm_init();
    start_idle();

    preempt_enable();
    first_user_task();

    while(1)
        hlt();
}
