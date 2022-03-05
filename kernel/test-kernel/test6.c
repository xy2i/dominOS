/*******************************************************************************
 * Test 6
 *
 * Waitpid multiple.
 * Creation de processus avec differentes tailles de piles.
 *******************************************************************************/

#include "task.h"

#if defined microblaze
__asm__(".text\n"
	".globl proc6_1\n"
	"proc6_1:\n"
	"addik r3,r0,3\n"
	"rtsd r15,8\n"
	"nop\n"
	".previous\n");
#else
__asm__(".text\n"
	".globl proc6_1\n"
	"proc6_1:\n"
	"movl $3,%eax\n"
	"ret\n"
	".previous\n");
#endif
int proc6_1(void *arg);

#if defined microblaze
__asm__(".text\n"
	".globl proc6_2\n"
	"proc6_2:\n"
	"addk r3,r0,r5\n"
	"swi r3,r1,-4\n"
	"rtsd r15,8\n"
	"nop\n"
	".previous\n");
#else
__asm__(".text\n"
	".globl proc6_2\n"
	"proc6_2:\n"
	"movl 4(%esp),%eax\n"
	"pushl %eax\n"
	"popl %eax\n"
	"ret\n"
	".previous\n");
#endif
int proc6_2(void *arg);

#if defined microblaze
__asm__(".text\n"
	".globl proc6_3\n"
	"proc6_3:\n"
	"addk r3,r0,r5\n"
	"swi r3,r1,-4\n"
	"rtsd r15,8\n"
	"nop\n"
	".previous\n");
#else
__asm__(".text\n"
	".globl proc6_3\n"
	"proc6_3:\n"
	"movl 4(%esp),%eax\n"
	"pushl %eax\n"
	"popl %eax\n"
	"ret\n"
	".previous\n");
#endif
int proc6_3(void *arg);

int test6_main(void *args)
{
    int pid1, pid2, pid3;
    int ret;
    (void)args;

    assert(getprio(getpid()) == 128);
    pid1 = start(proc6_1, 0, 64, "proc6_1", 0);
    assert(pid1 > 0);
    pid2 = start(proc6_2, 4, 66, "proc6_2", (void *)4);
    assert(pid2 > 0);
    pid3 = start(proc6_3, 0xffffffff, 65, "proc6_3", (void *)5);
    assert(pid3 < 0);
    pid3 = start(proc6_3, 8, 65, "proc6_3", (void *)5);
    assert(pid3 > 0);
    assert(waitpid(-1, &ret) == pid2);
    assert(ret == 4);
    assert(waitpid(-1, &ret) == pid3);
    assert(ret == 5);
    assert(waitpid(-1, &ret) == pid1);
    assert(ret == 3);
    assert(waitpid(pid1, 0) < 0);
    assert(waitpid(-1, 0) < 0);
    assert(waitpid(getpid(), 0) < 0);
    printf("ok.\n");
    return 0;
}
