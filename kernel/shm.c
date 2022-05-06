/**
 * Shared memory page management.
 */
#include "stddef.h"
#include "stdint.h"
#include "hash.h"
#include "shm.h"
#include "mem.h"
#include "page_allocator.h"
#include "paging.h"
#include "task.h"
#include "string.h"

#define NB_PAGES 1
#define PAGE_MAX 256

#define SHARED_START 0xfff00000
#define SHARED_END 0xffffffff

/**
 * Virtual address allocator
 */

// 0 if it's not allocated, else 1
int virtual_map[PAGE_MAX] = { 0 };

void *allocate_memory()
{
    for (int i = 0; i < PAGE_MAX; i++) {
        if (virtual_map[i] == 0) {
            virtual_map[i] = 1;
            return (void *)(SHARED_START + PAGE_SIZE * i);
        }
    }
    return NULL;
}

void free_memory(void *addr)
{
    unsigned int index = ((uint32_t)addr - SHARED_START) / PAGE_SIZE;
    virtual_map[index] = 0;
}

/**
 * Shared memory
 */

/*
 * Range of shared pages:
 *
 * 0xFFF00000 -> 0xffffffff
 *
 * Number of shared pages: 256
 */

struct shp {
    void *physical_address;
    void *virtual_address;
    // When refcount == 0, this page is freed
    uint64_t refcount;
    char    *key;
};

/**
 * Mapping from a shared page key (its string) to the shared page.
 */
hash_t shp_table;

void shm_init()
{
    hash_init_string(&shp_table);
}

void *shm_create(const char *key)
{
    // A pointer to the key will be stored in shm.
    // However, the pointer we got was a virtual memory one, which will not
    // be valid when we switch processes.
    // Copy the string to kernel space first.
    char *key_alloc = mem_alloc((strlen(key) + 1) * sizeof(char));
    memcpy(key_alloc, key, strlen(key) + 1);

    if (key == NULL)
        return NULL; // key is NULL
    if (hash_isset(&shp_table, (void *)key_alloc))
        return NULL; // page already exists;

    void *address         = alloc_physical_page(NB_PAGES);
    void *virtual_address = allocate_memory();
    if (virtual_address == NULL)
        return NULL; // out of memory

    struct shp *shp       = mem_alloc(sizeof(struct shp));
    shp->virtual_address  = virtual_address;
    shp->physical_address = address;
    shp->refcount         = 1;
    shp->key              = key_alloc;

    // map the virtual address
    map_zone((uint32_t *)current()->regs[CR3], (uint32_t)shp->virtual_address,
             (uint32_t)(shp->virtual_address + PAGE_SIZE - 1),
             (uint32_t)shp->physical_address,
             (uint32_t)(shp->physical_address + PAGE_SIZE - 1), RW | US);

    hash_set(&shp_table, (void *)key_alloc, shp);
    return virtual_address;
}

void *shm_acquire(const char *key)
{
    struct shp *shp = hash_get(&shp_table, (void *)key, NULL);
    if (shp == NULL)
        return NULL; // shp not registered

    shp->refcount++;
    // map the virtual address
    map_zone((uint32_t *)current()->regs[CR3], (uint32_t)shp->virtual_address,
             (uint32_t)(shp->virtual_address + PAGE_SIZE - 1),
             (uint32_t)shp->physical_address,
             (uint32_t)(shp->physical_address + PAGE_SIZE - 1), RW | US);
    return shp->virtual_address;
}

void shm_release(const char *key)
{
    struct shp *shp = hash_get(&shp_table, (void *)key, NULL);
    if (shp == NULL)
        return; // shp not registered

    // unmap the virtual address
    unmap_zone((uint32_t *)current()->regs[CR3], (uint32_t)shp->virtual_address,
               (uint32_t)(shp->virtual_address + PAGE_SIZE - 1));

    shp->refcount--;
    if (shp->refcount == 0) {
        // no more refs, free the page now
        free_physical_page(shp->physical_address, NB_PAGES);
        free_memory(shp->virtual_address);
        mem_free(shp->key, strlen(key) + 1);
        mem_free(shp, sizeof(struct shp));
    }
}