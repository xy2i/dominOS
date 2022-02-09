#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <stdint.h>

/**
 * Initialize the clock subsystem.
 * This activates the clock, and runs the
 * `tic_PIT()`
 * function defined in clock.c every tick.
 */
void init_clock();

uint32_t current_clock();

#endif //__CLOCK_H__