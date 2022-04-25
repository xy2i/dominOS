#ifndef __CGA_H__
#define __CGA_H__

#include <stddef.h>
#include <stdint.h>
/*
 * CGA colors definition.
 */

#define BLINK 0X80

#define BLACK_FG 0x00
#define BLUE_FG 0x01
#define GREEN_FG 0x02
#define CYAN_FG 0x03
#define RED_FG 0x04
#define MAGENTA_FG 0x05
#define BROWN_FG 0x06
#define GREY_FG 0x07
#define DARK_GREY_FG 0x08
#define LIGHT_BLUE_FG 0x09
#define LIGHT_GREEN_FG 0x0A
#define LIGHT_CYAN_FG 0x0B
#define LIGHT_RED_FG 0x0C
#define LIGHT_MAGENTA_FG 0x0D
#define LIGHT_YELLOW_FG 0X0E
#define LIGHT_WHITE_FG 0x0F

#define BLACK_BG 0X00
#define BLUE_BG 0x10
#define GREEN_BG 0x20
#define CYAN_BG 0x30
#define RED_BG 0x40
#define MAGENTA_BG 0x50
#define BROWN_BG 0x60
#define GREY_BG 0x70

void console_putbytes(char *bytes, size_t len);
void console_putbytes_color(char *bytes, size_t len, uint8_t color);

void console_putbytes_topright(char *bytes, size_t len);

/**
 * Syscall: Write on the terminal.
 * @param str The address to write.
 * @param size Number of bytes to write.
 * @return 0
 */
int cons_write(const char *str, long size);

#endif