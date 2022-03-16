#ifndef __SYSAPI_H__
#define __SYSAPI_H__

#include <stdio.h>
#include <stddef.h>
#include "../primitive.h"
#include "string.h"

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

#endif