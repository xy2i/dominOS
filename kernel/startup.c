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
#include "primitive.h"

#define START_TEST(n)                                                          \
    do {                                                                       \
        printf("Starting test: " #n ".\n");                                    \
        start(test##n, 512, 128, "test" #n, NULL);                             \
        printf("Test " #n " successfull.\n");                                  \
    } while (0)
static int __attribute__((noreturn)) one(void *arg __attribute__((unused)))
{
    for (;;) {
        printf("1");
        sti();
        hlt();
        cli();
    }
}
//static int __attribute__((noreturn)) two(void *arg __attribute__((unused)))
//{
//    for (;;) {
//        printf("2");
//        sti();
//        hlt();
//        cli();
//    }
//}

void kernel_start(void)
{
    printf("\f");

    sti();
    //preempt_disable();
    init_clock();
    init_page_fault_handler();
    init_syscall_handler();
    shm_init();
    uapp_init();
    start_idle();
    preempt_enable();

    struct uapps *u = get_uapp_by_name("proc1");
    printf("u->start:%x, u->end:%x, u->name:%s\n", (int)u->start, (int)u->end,
           u->name);
    u = get_uapp_by_name("proc2");
    printf("u->start:%x, u->end:%x, u->name:%s\n", (int)u->start, (int)u->end,
           u->name);

    start(one, 4096, MIN_PRIO, "proc1", NULL);
    start(one, 4096, MIN_PRIO, "proc2", NULL);

    //preempt_enable();
    //first_user_task();
    //printf("%d\n", ggetprio(0)); // syscall

    while (1) {
        sti();
        hlt();
        cli();
    }
}
