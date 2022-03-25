#include "cpu.h"
#include "stdio.h"
#include "stdint.h"
#include "clock.h"
#include "start.h"
#include "task.h"
#include "shm.h"
#include "ktests.h"
#include "pid_allocator.h"

#define START_TEST(n)                                                          \
    do {                                                                       \
        printf("Starting test: " #n ".\n");                                    \
        start(test##n, 512, 128, "test" #n, NULL);                             \
    } while (0)

void kernel_start(void)
{
    printf("\f");
    preempt_disable();
    init_clock();
    shm_init();
    start_idle();
    preempt_enable();

    START_TEST(8);

    while (1)
        hlt();
}
