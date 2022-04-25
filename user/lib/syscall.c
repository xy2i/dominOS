/**
 * Syscall interface to call from C with types.
 */

#include "../../kernel/syscall_asm.h"

// Macros to define syscalls easily.
// These call the function with the right type and cast the return.
// The called assembly functions are in syscall_asm.S/.h .
//
// Example of macro expansion:
// int getprio(int pid)
// {
//     return (int)syscall_1(2, pid);
// }
// Inspired from http://www.jamesmolloy.co.uk/tutorial_html/10.-User%20Mode.html

#define DEF_SYSCALL0(num, TYPE_RETOUR, fn)                                     \
    TYPE_RETOUR fn()                                                           \
    {                                                                          \
        return (TYPE_RETOUR)syscall_0(num);                                    \
    }

#define DEF_SYSCALL1(num, TYPE_RETOUR, fn, T1, arg1)                           \
    TYPE_RETOUR fn(T1 arg1)                                                    \
    {                                                                          \
        return (TYPE_RETOUR)syscall_1(num, (int)arg1);                         \
    }

#define DEF_SYSCALL2(num, TYPE_RETOUR, fn, T1, arg1, T2, arg2)                 \
    TYPE_RETOUR fn(T1 arg1, T2 arg2)                                           \
    {                                                                          \
        return (TYPE_RETOUR)syscall_2(num, (int)arg1, (int)arg2);              \
    }

#define DEF_SYSCALL3(num, TYPE_RETOUR, fn, T1, arg1, T2, arg2, T3, arg3)       \
    TYPE_RETOUR fn(T1 arg1, T2 arg2, T3 arg3)                                  \
    {                                                                          \
        return (TYPE_RETOUR)syscall_3(num, (int)arg1, (int)arg2, (int)arg3);   \
    }

#define DEF_SYSCALL4(num, TYPE_RETOUR, fn, T1, arg1, T2, arg2, T3, arg3, T4,   \
                     arg4)                                                     \
    TYPE_RETOUR fn(T1 arg1, T2 arg2, T3 arg3, T4 arg4)                         \
    {                                                                          \
        return (TYPE_RETOUR)syscall_4(num, (int)arg1, (int)arg2, (int)arg3,    \
                                      (int)arg4);                              \
    }

#define DEF_SYSCALL5(num, TYPE_RETOUR, fn, T1, arg1, T2, arg2, T3, arg3, T4,   \
                     arg4, T5, arg5)                                           \
    TYPE_RETOUR fn(T1 arg1, T2 arg2, T3 arg3, T4 arg4)                         \
    {                                                                          \
        return (TYPE_RETOUR)syscall_5(num, (int)arg1, (int)arg2, (int)arg3,    \
                                      (int)arg4, (int)arg5);                   \
    }

#define DEF_SYSCALL6(num, TYPE_RETOUR, fn, T1, arg1, T2, arg2, T3, arg3, T4,   \
                     arg4, T5, arg5, T6, arg6)                                 \
    TYPE_RETOUR fn(T1 arg1, T2 arg2, T3 arg3, T4 arg4)                         \
    {                                                                          \
        return (TYPE_RETOUR)syscall_6(num, (int)arg1, (int)arg2, (int)arg3,    \
                                      (int)arg4, (int)arg5, (int)arg6);        \
    }

/*
 * Declare all the function wrappers for syscalls.
 * The syscall numbers are decided via user/lib/weak-syscall-stubs.S,
 * provided with the project: it's in the same order.
 */

DEF_SYSCALL4(0, int, start, const char, name, unsigned long, ssize, int, prio,
             void *, arg);
DEF_SYSCALL0(1, int, getpid);
DEF_SYSCALL1(2, int, getprio, int, pid);
DEF_SYSCALL2(3, int, chprio, int, pid, int, newprio);
DEF_SYSCALL1(4, int, kill, int, pid);
DEF_SYSCALL2(5, int, waitpid, int, pid, int *, retvalp);
/* Since exit() is noreturn in gcc, include a while(1); at the end. We code this manually */
void exit(int retval)
{
    syscall_1(6, retval);
    while (1)
        ;
}
DEF_SYSCALL2(7, void, cons_write, const char *, str, long, size);
DEF_SYSCALL2(8, unsigned long, cons_read, char *, string, unsigned long, length);
/*
#if defined CONS_READ_LINE
DEF_SYSCALL2(8, unsigned long, cons_read, char *, string, unsigned long, length);
#elif defined CONS_READ_CHAR
DEF_SYSCALL2(8, int, cons_read, char *, string, unsigned long, length);
#endif
*/
DEF_SYSCALL1(9, void, cons_echo, int, on);
// 10: scount not implemented
// 11: screate not implemented
// 12: sdelete not implemented
// 13: signal not implemented
// 14: signaln not implemented
// 15: sreset not implemented
// 16: try_wait not implemented
// 17: wait not implemented
DEF_SYSCALL2(18, int, pcount, int, fid, int *, count);
DEF_SYSCALL1(19, int, pcreate, int, count);
DEF_SYSCALL1(20, int, pdelete, int, fid);
DEF_SYSCALL2(21, int, precieve, int, fid, int *, message);
DEF_SYSCALL1(22, int, preset, int, fid);
DEF_SYSCALL2(23, void, clock_settings, unsigned long *, quartz, unsigned long *,
             ticks);
DEF_SYSCALL0(24, unsigned long, current_clock);
DEF_SYSCALL0(25, void, sys_info);
DEF_SYSCALL1(26, void *, shm_create, const char *, key);
DEF_SYSCALL1(27, void *, shm_acquire, const char *, key);
DEF_SYSCALL1(28, void *, shm_release, const char *, key);
