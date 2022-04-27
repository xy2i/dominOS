/**
 * CGA screen manipulation (printing to screen).
 *
 * One important detail:
 * the cursor is drawn by the video card, using the foreground and background set at the
 * current cell in the video memory.
 * However, if we set the memory at a cell to only zeroes
 * then we will have a black cursor on a black foreground, which will not show up.
 *
 * Thus, we must take care to initialize the cell
 * that the pointer is on with white foreground on black background.
 *
 * To do so, we initialize all unwritten cells with a white foreground value.
 */
#include <string.h>
#include "cga.h"
#include "cpu.h"
#include "debug.h"

#define BASE_MEM_ADDR 0xB8000
#define NUMBER_COLUMN 80
#define NUMBER_LINE 25

/*
 * VRAM related definitions: get a pointer to a cell of video memory.
 */
#define MEM_VIDEO_OFFSET(LINE, COLUMN) (LINE * NUMBER_COLUMN + COLUMN)
#define PTR_MEM(LINE, COLUMN) ((uint16_t *)(BASE_MEM_ADDR + 2 * MEM_VIDEO_OFFSET(LINE, COLUMN)))

/*
 * Cursor related IO ports.
 */
#define CURSOR_CMD_PORT 0x3D4
#define CURSOR_DATA_PORT 0x3D5
/*
 * Port commands.
 */
#define CURSOR_WRITE_LOW 0x0F
#define CURSOR_WRITE_HIGH 0x0E
#define WHITE_ON_BLACK 0x0F00

static uint8_t cur_line = 0;
static uint8_t cur_column = 0;

void write_char(uint8_t line, uint8_t column, char c, uint8_t color)
{
    uint16_t opts = (0 << 7) | color;
    uint16_t volatile * const memory_address = PTR_MEM(line, column);
    *memory_address = opts << 8 | c;
}

static void put_cursor(uint8_t line, uint8_t column)
{
    cur_line = line;
    cur_column = column;
    uint16_t cursor_pos = column + line * 80;

    outb(CURSOR_WRITE_LOW, CURSOR_CMD_PORT);
    outb(cursor_pos & 0xFF, CURSOR_DATA_PORT);
    outb(CURSOR_WRITE_HIGH, CURSOR_CMD_PORT);
    outb((cursor_pos >> 8) & 0xFF, CURSOR_DATA_PORT);
}

void clear_screen()
{
    size_t count = NUMBER_COLUMN * NUMBER_LINE;
    uint16_t* buf = PTR_MEM(0, 0);
    while(count--) *buf++ = WHITE_ON_BLACK;
}

static void scroll_screen()
{
    memmove((void *) BASE_MEM_ADDR, PTR_MEM(1, 0), 2 * (NUMBER_LINE - 1) * NUMBER_COLUMN);

    size_t count = NUMBER_COLUMN;
    uint16_t* buf = PTR_MEM((NUMBER_LINE - 1), 0);
    while(count--) *buf++ = WHITE_ON_BLACK;
}


static void console_putchar(char c, uint8_t color)
{
    switch (c) {
        case '\b':
            if (cur_column != 0) cur_column--;
            break;
        case '\t': {
            uint8_t cur_tab = cur_column & 0b11111000; // % 8
            cur_column = cur_tab == 72 ? 79 : cur_tab | 0b111;
            break;
        }
        case '\n':
            cur_line++;
            cur_column = 0;
            break;
        case '\f':
            clear_screen();
            cur_column = cur_line = 0;
            break;
        case '\r':
            cur_column = 0;
            break;
        default:
            if((int)c == 127){ //DEBUG
                if (cur_column != 0){
                    cur_column--;
                }else{
                    cur_line--;
                    cur_column = 79;
                }
                *PTR_MEM(cur_line, cur_column) = WHITE_ON_BLACK;
                break;
            }
            write_char(cur_line, cur_column, c, color);
            if (cur_column == NUMBER_COLUMN - 1) { // Wrote on last column
                cur_line++;
                cur_column = 0;
            } else {
                cur_column++;
            }
    }

    if (cur_line == NUMBER_LINE) {
        scroll_screen();
        cur_line--;
    }
}

void console_putbytes_color(char *bytes, size_t len, uint8_t color) {
    for (size_t i = 0; i < len; i++) {
        console_putchar(bytes[i], color);
    }
    put_cursor(cur_line, cur_column);
}

void console_putbytes(char *bytes, size_t len) {
    console_putbytes_color(bytes, len, LIGHT_WHITE_FG);
}

void console_putbytes_topright(char *bytes, size_t len) {
    put_cursor(0, NUMBER_COLUMN - len);
    console_putbytes(bytes, len);
    put_cursor(cur_line, cur_column);
}

void cons_write(const char *str, long size)
{
    console_putbytes((char *)str, size);
}