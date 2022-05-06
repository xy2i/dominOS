#include <stdio.h>
#include <string.h>
#include <primitive.h>
#include "shell.h"

#define BUFF_SIZE 50

int main()
{
    display_title();

    while (1) {
        change_color(BLACK_FG | GREEN_BG);
        printf("ensimag");
        change_color(DEFAULT);
        printf("@");
        change_color(RED_FG);
        printf("dominos> ");
        change_color(DEFAULT);
        char buff[BUFF_SIZE];
        memset(buff, 0, BUFF_SIZE);
        unsigned long cmd_size = cons_read(buff, 50);
        (void)cmd_size;

        if (strcmp(buff, "help") == 0) {
            printf("help: Show all the command you can type\n"
                   "ps: display information about all process\n"
                   "exit: Exit the shell\n");
        } else if (strcmp(buff, "ps") == 0) {
            ps();
        } else if (strcmp(buff, "exit") == 0) {
            printf("Goodbye !\n");
            return 0;
        } else {
            printf("Incorrect Command, type help to see commands\n");
        }
    }
}