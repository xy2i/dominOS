#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include "segment.h"
void register_interrupt_handler(int number, void (*handler)(void));

#endif