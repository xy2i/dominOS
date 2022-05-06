#include <stdio.h>
#include <string.h>
#include <primitive.h>
#include "shell.h"

#define BUFF_SIZE 50

void prompt_retval(int retval)
{
    change_color(retval == 0 ? GREEN_FG : RED_FG);
    printf("[%d] ", retval);
    change_color(DEFAULT);
}

void prompt()
{
    change_color(BLACK_FG | GREEN_BG);
    printf("ensimag");
    change_color(DEFAULT);
    printf("@");
    change_color(RED_FG);
    printf("dominos> ");
    change_color(DEFAULT);
}

int main()
{
    display_title();

    while (1) {
        prompt();

        char buff[BUFF_SIZE];
        memset(buff, 0, BUFF_SIZE);
        unsigned long cmd_size = cons_read(buff, 50);
        (void)cmd_size;

        if (strcmp(buff, "help") == 0) {
            printf("process_name: Start the process `process_name`\n"
                   "autotest: Run all tests\n"
                   "help: Show all the command you can type\n"
                   "ps: display information about all process\n"
                   "exit: Exit the shell\n");
        } else if (strcmp(buff, "ps") == 0) {
            ps();
        } else if (strcmp(buff, "exit") == 0) {
            printf("Goodbye !\n");
            return 0;
        } else {
            // Try to launch a process first if it exists
            int pid = start(buff, 4096, 128, NULL);
            if (pid < 0) {
                printf("Incorrect command %s, type help to see commands\n",
                       buff);
                continue;
            }

            int retval;
            waitpid(pid, &retval);
            prompt_retval(retval);
        }
    }
}