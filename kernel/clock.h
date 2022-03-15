#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <stdint.h>

void     set_clock_frequency(uint32_t hz);
void     init_clock(void);
void     clock_settings(unsigned long * quartz, unsigned long * ticks);
uint32_t current_clock(void);

#endif
