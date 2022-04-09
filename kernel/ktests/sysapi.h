#ifndef __SYSAPI_H__
#define __SYSAPI_H__

#include <stdio.h>
#include <stddef.h>
#include "../primitive.h"
#include "string.h"

#define WITH_MSG

/* it.c */
void test_it(void);
/* math.h */
typedef unsigned long long uint_fast64_t;
typedef unsigned long uint_fast32_t;
short randShort(void);
void setSeed(uint_fast64_t _s);
unsigned long rand();
unsigned long long div64(unsigned long long num, unsigned long long div,
			 unsigned long long *rem);

/* Wrapper sur les verrous basés sur les sémaphores ou les files de messages */
union sem {
    int fid;
    int sem;
};

void xwait(union sem *s);
void xsignal(union sem *s);
void xsdelete(union sem *s);
void xscreate(union sem *s);

#endif
