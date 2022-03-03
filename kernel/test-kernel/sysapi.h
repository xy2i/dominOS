/*
 * Ensimag - Projet système
 * Copyright (C) 2012 - Damien Dejean <dam.dejean@gmail.com>
 *
 * Unique header of the standalone test standard library.
 */

#ifndef _SYSAPI_H_
#define _SYSAPI_H_

#define NULL ((void *)0)

/*******************************************************************************
 * Assert : check a condition or fail
 ******************************************************************************/
#define __STRING(x) #x

/*#define assert(cond)                                                           \
    ((void)((cond) ? 0 : assert_failed(__STRING(cond), __FILE__, __LINE__)))*/

#define DUMMY_VAL 78

#define TSC_SHIFT 8

#define FREQ_PREC 50

#define NBSEMS 10000

#define TRUE 1
#define FALSE 0

#define NR_PHILO 5

/* Available from our standard library */
#ifndef __SIZE_TYPE__
#error __SIZE_TYPE__ not defined
#endif

typedef __SIZE_TYPE__ size_t;

int strcmp(const char *str1, const char *str2);
size_t strlen(const char *s);
char *strncpy(char *dst, const char *src, unsigned n);
void *memset(void *dst, int c, size_t n);

/* printf.h */
//#define printf safe_printf
//int safe_printf(const char *format, ...);
//void cons_gets(char *s, unsigned long length);

/* assert.c */
//int assert_failed(const char *cond, const char *file, int line);

/* math.h */
typedef unsigned long long uint_fast64_t;
typedef unsigned long uint_fast32_t;
short randShort(void);
void setSeed(uint_fast64_t _s);
unsigned long rand();
unsigned long long div64(unsigned long long num, unsigned long long div,
			 unsigned long long *rem);

/* it.c */
void test_it(void);

#endif /* _SYSAPI_H_ */
