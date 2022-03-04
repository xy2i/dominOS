/*******************************************************************************
 * Test Start With Args
 *
 * Tests the passing of args to when starting a new process.
 ******************************************************************************/

#include "stdio.h"
#include "test_start_with_args.h"

int test_start_with_args_main(void *args)
{
    struct point *p = (struct point *) args;
    printf("x: %i y: %i\n", p->x, p->y);

    return 0;
}
