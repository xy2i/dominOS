#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <stdint.h>

/**
 * Initialize the clock subsystem.
 * This activates the clock interrupt handler.
 */
void init_clock();

uint32_t current_clock();

#endif //__CLOCK_H__