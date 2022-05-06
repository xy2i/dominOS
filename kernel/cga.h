#ifndef __CGA_H__
#define __CGA_H__

#include <stddef.h>
#include <stdint.h>
#include "cga_colors.h"

void console_putbytes(char *bytes, size_t len);
void console_putbytes_color(char *bytes, size_t len, uint8_t color);

void console_putbytes_topright(char *bytes, size_t len);

void remove_last(void);

#endif