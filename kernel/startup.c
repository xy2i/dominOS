#include "cpu.h"
#include "stdio.h"
#include "stdint.h"
#include "clock.h"
#include "kbd.h"
#include "start.h"
#include "task.h"
#include "shm.h"
#include "page_allocator.h"
#include "ktests.h"
#include "usermode.h"
#include "syscall_handler_init.h"
#include "userspace_apps.h"
#include "paging.h"
#include "primitive.h"

#define START_TEST(n)                                                          \
    do {                                                                       \
        printf("Starting test: " #n ".\n");                                    \
        start(test##n, 512, 128, "test" #n, NULL);                             \
        printf("Test " #n " successfull.\n");                                  \
    } while (0)

void kernel_start(void)
{
    printf("\f");

    init_clock();
    init_keyboard_handler();
    //init_page_fault_handler();
    init_syscall_handler();
    preempt_enable();
    shm_init();
    uapp_init();
    start_idle();
    preempt_enable();

    start("proc2", 4096, MIN_PRIO, NULL);

    while (1) {
        sti();
        hlt();
        cli();
    }
}
