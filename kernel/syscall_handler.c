#include "primitive.h"
#include "interrupts.h"
#include "isr.h"
#include "syscall_handler.h"
#include <stdio.h>

void no_impl()
{
    register int eax __asm__("eax");
    printf("warning: syscall %d not implemented yet", eax);
}

/**
 * Map each syscall number to a function that does the syscall.
 * See syscall.c for all declared syscalls.
 * When you define a syscall, replace no_impl with the implementation
 * of the syscall.
 */
void *syscalls[NUM_SYSCALLS] = {
    [0] = start,
    [1] = getpid,
    [2] = getprio,
    [3] = chprio,
    [4] = kill,
    [5] = waitpid,
    [6] = exit,
    [7] = cons_write,
    [8] = cons_read,
    [9] = cons_echo,
    [10] = no_impl,
    [11] = no_impl,
    [12] = no_impl,
    [13] = no_impl,
    [14] = no_impl,
    [15] = no_impl,
    [16] = no_impl,
    [17] = no_impl,
    [18] = pcount,
    [19] = pcreate,
    [20] = pdelete,
    [21] = preceive,
    [22] = preset,
    [23] = psend,
    [24] = clock_settings,
    [25] = current_clock,
    [26] = wait_clock,
    [27] = no_impl,
    [28] = shm_create,
    [29] = shm_acquire,
    [30] = shm_release,
    [31] = halt,
    [32] = ps,
    [33] = change_color,
};

/**
 * See syscall_asm.S, syscall_isr for the syscall handler
 */
void init_syscall_handler(void)
{
    num_syscalls = NUM_SYSCALLS;
    register_interrupt_handler(49, syscall_isr);
}
