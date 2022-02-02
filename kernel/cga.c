#include <stdint.h>
#include "cga.h"

void write_char(uint8_t line, uint8_t column, char c, uint8_t color)
{
    uint16_t volatile * const memory_address = VIDEO_MEM_PTR(line, column);
    *memory_address = MAKE_CELL(c, color);
}