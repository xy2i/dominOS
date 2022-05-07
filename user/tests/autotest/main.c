/*
 * Ensimag - Projet système
 * Copyright (C) 2013 - Damien Dejean <dam.dejean@gmail.com>
 */

#include "sysapi.h"

#define TESTS_NUMBER 23

const char *tests[TESTS_NUMBER] = {
    "test0",  "test1",  "test2",  "test3",  "test4",  "test5",
    "test6",  "test7",  "test8",  "test9",  "test10", "test11",
    "test12", "test13", "test14", "test15", "test16", "test17",
    "test18", "test19", "test20", "test21", "test22",
};

extern void change_color(unsigned char color);
#define RED_FG 0x04
#define LIGHT_WHITE_FG 0x0F
#define DEFAULT LIGHT_WHITE_FG

int main(void)
{
    int i;
    int pid;
    int ret;

    for (i = 0; i < TESTS_NUMBER; i++) {
        printf("Test %s : ", tests[i]);
        pid = start(tests[i], 4000, 128, NULL);
        waitpid(pid, &ret);
        if (ret != 0) {
            assert(ret != 0);
        }
    }
}
