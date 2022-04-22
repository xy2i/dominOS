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
#include "userspace_apps.h"
#include "paging.h"
#include "syscall.h"

#define START_TEST(n)                                                          \
    do {                                                                       \
        printf("Starting test: " #n ".\n");                                    \
        start(test##n, 512, 128, "test" #n, NULL);                             \
        printf("Test " #n " successfull.\n");                                  \
    } while (0)

void kernel_start(void)
{
    printf("\f");

    sti();
    preempt_disable();
    //init_clock();
    init_page_fault_handler();
    init_syscall_handler();
    shm_init();
    uapp_init();
    //start_idle();

    initialise_paging();
    printf("after paging!\n");
    printf("content of address 0x0: %d\n", *((uint32_t *)0));
    printf("write to 0x0?\n");
    *((uint32_t *)0) = 1;
    printf("succesful, contents: %d", *(uint32_t *)0);
    int i = 0;
    while (1) {
        i++;
    }
    //preempt_enable();
    //first_user_task();
    //printf("%d\n", ggetprio(0)); // syscall

    while (1)
        hlt();
}
