/*******************************************************************************
 * Test 7
 *
 * Test de l'horloge (ARR et ACE)
 * Tentative de determination de la frequence du processeur et de la
 * periode de scheduling
 ******************************************************************************/

#include "sysapi.h"
#include "task.h"
#include "shm.h"
#include "clock.h"
#include "test7.h"

int timer_f(void *arg)
{
    volatile unsigned long *timer = NULL;
    timer = shm_acquire("test7_shm");
    assert(timer != NULL);

    (void)arg;
    while (1) {
	unsigned long t = *timer + 1;
	*timer = t;
	while (*timer == t)
	    test_it();
    }
    while (1)
	;
    return 0;
}

int timer1(void *arg)
{
    (void)arg;

    unsigned long quartz;
    unsigned long ticks;
    unsigned long dur;
    int i;

    clock_settings(&quartz, &ticks);
    dur = (quartz + ticks) / ticks;
    printf(" 2");
    for (i = 4; i < 8; i++) {
	wait_clock(current_clock() + dur);
	printf(" %d", i);
    }
    return 0;
}

int sleep_pr1(void *arg)
{
    (void)arg;
    wait_clock(current_clock() + 2);
    printf(" not killed !!!");
    assert(0);
    return 1;
}

void test7_main(void)
{
    int pid1, pid2, r;
    unsigned long c0, c, quartz, ticks, dur;
    volatile unsigned long *timer = NULL;

    timer = shm_create("test7_shm");
    assert(timer != NULL);

    assert(getprio(getpid()) == 128);
    printf("1");
    pid1 = start(timer1, 4000, 129, "timer1", 0);
    assert(pid1 > 0);
    printf(" 3");
    assert(waitpid(-1, 0) == pid1);
    printf(" 8 : ");

    *timer = 0;
    pid1 = start(timer_f, 4000, 127, "timer", 0);
    pid2 = start(timer_f, 4000, 127, "timer", 0);
    assert(pid1 > 0);
    assert(pid2 > 0);
    clock_settings(&quartz, &ticks);
    dur = 2 * quartz / ticks;
    test_it();
    c0 = current_clock();
    do {
	test_it();
	c = current_clock();
    } while (c == c0);
    wait_clock(c + dur);
    assert(kill(pid1) == 0);
    assert(waitpid(pid1, 0) == pid1);
    assert(kill(pid2) == 0);
    assert(waitpid(pid2, 0) == pid2);
    printf("%lu changements de contexte sur %lu tops d'horloge", *timer, dur);
    pid1 = start(sleep_pr1, 4000, 192, "sleep_pr1", 0);
    assert(pid1 > 0);
    assert(kill(pid1) == 0);
    assert(waitpid(pid1, &r) == pid1);
    assert(r == 0);
    printf(".\n");
    shm_release("test7_shm");
}
