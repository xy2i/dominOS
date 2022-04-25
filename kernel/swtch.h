#ifndef __SWTCH_H__
#define __SWTCH_H__

#include <stdint.h>

struct cpu_context {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
};

void swtch(struct cpu_context **old, struct cpu_context *new,
           uint32_t *new_page_dir, uint32_t *stack_addr);

#endif
