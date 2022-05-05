#define WITH_MSG
#define CONS_READ_LINE
#include "../tests/lib/sysapi.h"

#define BUFF_SIZE 50

// ps a passer coté kernel avec appel systeme
/*int ps(){
    struct task *p;
    char *s = "\npid\tprio\tstate\tnext\tname\n";
    cons_write(s, sizeof(s));
    queue_for_each(p, get_all_tasks(), struct task, global_tasks)
    {
        s = (char*)p->pid;
        cons_write(s, sizeof(s));
        s = (char*)p->priority;
        cons_write(s, sizeof(s));

        switch (p->state) {
            case TASK_RUNNING:
                s = "run";
                break;
            case TASK_READY:
                s = "ready";
                break;
            case TASK_SLEEPING:
                s = "sleep";
                break;
            case TASK_ZOMBIE:
                s = "zombie";
                break;
            case TASK_INTERRUPTED_MSG:
                s = "in msg";
                break;
            case TASK_INTERRUPTED_CHILD:
                s = "child";
                break;
            //default:
                //s = (char*)p->state;
        }
        cons_write(s, sizeof(s));

        *//*
        if (p->global_tasks.next != 0) {
            printf("%d\t",
                   queue_entry(p->global_tasks.next, struct task, global_tasks)
                       ->pid);
        } else {
            printf("EOL\t");
        }
        printf("\t%s", p->comm);
        printf("\n");
        *//*
    }
    return 0;
}*/

int main(){
    cons_write("Welcome to our OS ! Tap help to see the command you can make\n",
               sizeof("Welcome to our OS ! Tap help to see the command you can make\n"));

    while(1){
        cons_write("ensimag@linux> ", sizeof("ensimag@linux> "));
        char buff[BUFF_SIZE];
        memset(buff, 0, BUFF_SIZE);
        unsigned long cmd_size = cons_read(buff, 50);
        (void) cmd_size;
        if(strcmp(buff, "help") == 0) {
            cons_write(" help: Show all the command you can type\n",
                       sizeof(" help: Show all the command you can type\n"));
            cons_write("ps: display information about all process\n",
                       sizeof("ps: display information about all process\n"));
        }
        else if(strcmp(buff, "ps") == 0) {
            cons_write(" Exectue ps command\n", sizeof("Exectue ps command\n"));
        }
        else {
            cons_write(" Incorrect Command, type help to see commands\n",
                       sizeof("Incorrect Command, type help to see commands\n"));
        }
        return 0;
    }
}