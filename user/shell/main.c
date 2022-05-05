#define WITH_MSG
#define CONS_READ_LINE
#include "../tests/lib/sysapi.h"
#include "shell.h"

#define BUFF_SIZE 50



int main(){
    display_title();

    while(1){
        cons_write("ensimag@domingos> ", sizeof("ensimag@linux> "));
        char buff[BUFF_SIZE];
        memset(buff, 0, BUFF_SIZE);
        unsigned long cmd_size = cons_read(buff, 50);
        (void) cmd_size;

        if(strcmp(buff, "help") == 0) {
            cons_write(" help: Show all the command you can type\n",
                       sizeof(" help: Show all the command you can type\n"));
            cons_write("ps: display information about all process\n",
                       sizeof("ps: display information about all process\n"));
            cons_write("exit: Exit the shell\n",
                       sizeof("exit: Exit the shell\n"));
        }
        else if(strcmp(buff, "ps") == 0) {
            ps();
        }
        else if(strcmp(buff, "exit") == 0) {
            cons_write("Goodbye !\n", sizeof("Goodbye !\n"));
            return 0;
        }
        else {
            cons_write(" Incorrect Command, type help to see commands\n",
                       sizeof("Incorrect Command, type help to see commands\n"));
        }
    }
}