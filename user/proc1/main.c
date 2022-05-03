#define WITH_MSG
#define CONS_READ_LINE
#include "../tests/lib/sysapi.h"

int main()
{
    cons_write("Hello from virtual address process\n",
               sizeof("Hello from virtual address process\n"));
    cons_write("Tap a word: ", sizeof("Tap a word: "));
    char buff[50];
    unsigned long i = cons_read(buff, 50);
    cons_write("\nthe word you tape is: ", sizeof("\nthe word you tape is: "));
    cons_write(buff, i);
    cons_write("\n", sizeof("\n"));
    cons_write("Tap a word: ", sizeof("Tap a word: "));
    char buff2[50];
    unsigned long i2 = cons_read(buff2, 50);
    cons_write("\nthe word you tape is: ", sizeof("\nthe word you tape is: "));
    cons_write(buff2, i2);
    cons_write("\n", sizeof("\n"));
    cons_write("Tap a word: ", sizeof("Tap a word: "));
    char buff3[50];
    unsigned long i3 = cons_read(buff3, 50);
    cons_write("\nthe word you tape is: ", sizeof("\nthe word you tape is: "));
    cons_write(buff3, i3);
    cons_write("\n", sizeof("\n"));
    return 0;
}