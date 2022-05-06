#include "primitive.h"
#include <stdio.h>

int main()
{
    printf("Hello from virtual address process\n");
    printf("Tap a word: ");
    char          buff[50];
    unsigned long i = cons_read(buff, 50);
    printf("\nthe word you tape is: ");
    cons_write(buff, i);
    printf("\n");
    printf("Tap a word: ");
    char          buff2[50];
    unsigned long i2 = cons_read(buff2, 50);
    printf("\nthe word you tape is: ");
    cons_write(buff2, i2);
    printf("\n");
    printf("Tap a word: ");
    char          buff3[50];
    unsigned long i3 = cons_read(buff3, 50);
    printf("\nthe word you tape is: ");
    cons_write(buff3, i3);
    printf("\n");
    return 0;
}