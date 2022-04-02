#include "sysapi.h"

int main()
{
    __asm__("int $49");
    __asm__("int $49");
    for (;;)
	;
}