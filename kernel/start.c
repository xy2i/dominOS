#include "debugger.h"
#include "cpu.h"
#include "stdio.h"
#include "stdint.h"
#include "cga.h"

int fact(int n)
{
	if (n < 2)
		return 1;

	return n * fact(n-1);
}


void kernel_start(void)
{
//	int i;
//	call_debugger();

	//i = 10;

	//i = fact(i);

    printf("\f");
    printf("Clearing the screen: \f");
    printf("This is a little test\n");
    printf("Some tabulation: \t1\t2\t3\t4\t5\t6\t7\t8\t9\t0\n");
    printf("This is best\rI am the\n");
    printf("Hang on i made a mistake\b\b\b\b\b\b\bsuccess!\n");
    printf("Lorem ipsum Lorem ipsum Lorem ipsum Lorem ipsum Lorem ipsum Lorem ipsum Lorem ipsum Lorem ipsum Lorem ipsum \n");

    for (uint8_t i = 0; i < 18; i++) {
        printf("I'm scrolling now wheeeeeeee\n");
    }

    console_putbytes_topright("abcde", 5);

    
    while (1)
        hlt();
}
