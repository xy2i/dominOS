/**
 * Syscall interface to call from C with types.
 */

// Macros to define syscalls easily.
// These call the function with the right type and cast the return.
//
// Inspired from http://www.jamesmolloy.co.uk/tutorial_html/10.-User%20Mode.html
// GCC inline asm tutorial: https://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html
// asm (asm code : output regs :input regs)
// "=a" ret and "0": we use both eax for num (as input) and ret (as output). 0 means the first
// constraint.

#define DEF_SYSCALL0(num, TYPE_RETOUR, fn)                                     \
    TYPE_RETOUR fn()                                                           \
    {                                                                          \
        int ret;                                                               \
        __asm__ volatile("int $49" : "=a"(ret) : "0"(num));                    \
        return (TYPE_RETOUR)ret;                                               \
    }

#define DEF_SYSCALL1(num, TYPE_RETOUR, fn, T1, arg1)                           \
    TYPE_RETOUR fn(T1 arg1)                                                    \
    {                                                                          \
        int ret;                                                               \
        __asm__ volatile("int $49" : "=a"(ret) : "0"(num), "b"((int)arg1));    \
        return (TYPE_RETOUR)ret;                                               \
    }

#define DEF_SYSCALL2(num, TYPE_RETOUR, fn, T1, arg1, T2, arg2)                 \
    TYPE_RETOUR fn(T1 arg1, T2 arg2)                                           \
    {                                                                          \
        int ret;                                                               \
        __asm__ volatile("int $49"                                             \
                         : "=a"(ret)                                           \
                         : "0"(num), "b"((int)arg1), "c"((int)arg2));          \
        return (TYPE_RETOUR)ret;                                               \
    }

#define DEF_SYSCALL3(num, TYPE_RETOUR, fn, T1, arg1, T2, arg2, T3, arg3)       \
    TYPE_RETOUR fn(T1 arg1, T2 arg2, T3 arg3)                                  \
    {                                                                          \
        int ret;                                                               \
        __asm__ volatile("int $49"                                             \
                         : "=a"(ret)                                           \
                         : "0"(num), "b"((int)arg1), "c"((int)arg2),           \
                           "d"((int)arg3));                                    \
        return (TYPE_RETOUR)ret;                                               \
    }

#define DEF_SYSCALL4(num, TYPE_RETOUR, fn, T1, arg1, T2, arg2, T3, arg3, T4,   \
                     arg4)                                                     \
    TYPE_RETOUR fn(T1 arg1, T2 arg2, T3 arg3, T4 arg4)                         \
    {                                                                          \
        int ret;                                                               \
        __asm__ volatile("int $49"                                             \
                         : "=a"(ret)                                           \
                         : "0"(num), "b"((int)arg1), "c"((int)arg2),           \
                           "d"((int)arg3), "S"((int)arg4));                    \
        return (TYPE_RETOUR)ret;                                               \
    }

#define DEF_SYSCALL5(num, TYPE_RETOUR, fn, T1, arg1, T2, arg2, T3, arg3, T4,   \
                     arg4, T5, arg5)                                           \
    TYPE_RETOUR fn(T1 arg1, T2 arg2, T3 arg3, T4 arg4)                         \
    {                                                                          \
        int ret;                                                               \
        __asm__ volatile("int $49"                                             \
                         : "=a"(ret)                                           \
                         : "0"(num), "b"((int)arg1), "c"((int)arg2),           \
                           "d"((int)arg3), "S"((int)arg4), "D"((int)arg5));    \
        return (TYPE_RETOUR)ret;                                               \
    }

/*
 * Declare all the function wrappers for syscalls.
 * The syscall numbers are decided via user/lib/weak-syscall-stubs.S,
 * provided with the project: it's in the same order.
 */

DEF_SYSCALL4(0, int, start, const char *, name, unsigned long, ssize, int, prio,
             void *, arg);
DEF_SYSCALL0(1, int, getpid);
DEF_SYSCALL1(2, int, getprio, int, pid);
DEF_SYSCALL2(3, int, chprio, int, pid, int, newprio);
DEF_SYSCALL1(4, int, kill, int, pid);
DEF_SYSCALL2(5, int, waitpid, int, pid, int *, retvalp);
/* Since exit() is noreturn in gcc, include a while(1); at the end. We code this manually */
void exit(int retval)
{
    int ret;
    __asm__ volatile("int $49" : "=a"(ret) : "0"(6), "b"((int)retval));
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
DEF_SYSCALL2(21, int, preceive, int, fid, int *, message);
DEF_SYSCALL1(22, int, preset, int, fid);
DEF_SYSCALL2(23, int, psend, int, fid, int, msg);
DEF_SYSCALL2(24, void, clock_settings, unsigned long *, quartz, unsigned long *,
             ticks);
DEF_SYSCALL0(25, unsigned long, current_clock);
DEF_SYSCALL1(26, void, wait_clock, unsigned long, clock);
DEF_SYSCALL0(27, void, sys_info);
DEF_SYSCALL1(28, void *, shm_create, const char *, key);
DEF_SYSCALL1(29, void *, shm_acquire, const char *, key);
DEF_SYSCALL1(30, void *, shm_release, const char *, key);
DEF_SYSCALL0(31, void, halt);
DEF_SYSCALL0(32, void, ps);
DEF_SYSCALL1(33, void, change_color, unsigned char, color);