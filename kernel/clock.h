#ifndef __CLOCK_H__
#define __CLOCK_H__

/**
 * Initialize the clock subsystem.
 * This activates the clock, and runs the
 * `tic_PIT()`
 * function defined in clock.c every tick.
 */
void init_clock();

#endif //__CLOCK_H__
