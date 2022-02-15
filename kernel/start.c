#include "debugger.h"
#include "cpu.h"
#include "stdio.h"
#include "stdint.h"
#include "cga.h"
#include "clock.h"
#include "task.h"
#include "test-kernel/test0.h"

void kernel_start(void) {

    printf("\f");

    // Init clock
    init_clock();

    // Init task
    init_tasks();

    // Enable interrupts
    sti();

    printf("[test0] ");
    create_kernel_task("test0", test0_main);

    while (1) hlt();
}
