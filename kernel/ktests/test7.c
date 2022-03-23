/*******************************************************************************
 * Test 7
 *
 * Test de l'horloge (ARR et ACE)
 * Tentative de determination de la frequence du processeur et de la
 * periode de scheduling
 ******************************************************************************/

#ifdef TELECOM_TST
int main(void *arg)
{
    (void)arg;
    printf("Test desactive pour les TELECOM.\n");
}

#else
#include "sysapi.h"

int timer1(void *arg);
int timer(void *arg);
int sleep_pr1(void *arg);

int test7(void *arg)
{
    int pid1, pid2, r;
    unsigned long c0, c, quartz, ticks, dur;
    volatile unsigned long *_timer = NULL;
    (void)arg;

    _timer = shm_create("test7_shm");
    assert(_timer != NULL);

    assert(getprio(getpid()) == 128);
    printf("1");
    pid1 = start(timer1, 4000, 129, "timer1", 0);
    assert(pid1 > 0);
    printf(" 3");
    assert(waitpid(-1, 0) == pid1);
    printf(" 8 : ");

    *_timer = 0;
    pid1 = start((int (*)(void *)) timer, 4000, 127, "timer", 0);
    pid2 = start((int (*)(void *)) timer, 4000, 127, "timer", 0);
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
    printf("%lu changements de contexte sur %lu tops d'horloge", *_timer, dur);
    pid1 = start(sleep_pr1, 4000, 192, "sleep_pr1", 0);
    assert(pid1 > 0);
    assert(kill(pid1) == 0);
    assert(waitpid(pid1, &r) == pid1);
    assert(r == 0);
    printf(".\n");
    shm_release("test7_shm");
    return 0;
}
#endif
