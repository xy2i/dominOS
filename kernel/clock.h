#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <stdint.h>

/**
 * Frequency of the clock in Hz.
 * Change this if you want to trigger ticks more/less frequently.
 */
#define CLOCK_FREQ 50
/**
 * Initialize the clock subsystem.
 * This activates the clock interrupt handler.
 */
void init_clock();
/**
 * Get the clock settings.
 */
void clock_settings(unsigned long *quartz, unsigned long *ticks);

uint32_t current_clock();

#endif //__CLOCK_H__
