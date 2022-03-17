#ifndef __PF_ALLOCATOR_H__
#define __PF_ALLOCATOR_H__

void * alloc_physical_page(void);
void   free_physical_page(void * physical_page)

#endif