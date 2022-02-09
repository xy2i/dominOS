#include "debugger.h"
#include "cpu.h"
#include "stdio.h"
#include "stdint.h"
#include "cga.h"
#include "clock.h"
#include "task.h"

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n-1);
}


void kernel_start(void) {
//	int i;
//	call_debugger();

    //i = 10;

    //i = fact(i);

    printf("\f");
    // Init clock
    init_clock();

    // Init task
    //init_task();

    while (1)
        hlt();
}
