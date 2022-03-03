/**
 * Shared memory page management.
 */
#include "stddef.h"
#include "stdint.h"
#include "hash.h"
#include "shm.h"
#include "mem.h"

#define PAGE_SIZE 4000

struct shp {
    void *address;
    // When refcount == 0, this page is freed
    uint64_t refcount;
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
    if (key == NULL)
	return NULL; // key is NULL
    if (hash_isset(&shp_table, key))
	return NULL; // page already exists;

    void *address = mem_alloc(PAGE_SIZE);
    if (address == NULL)
	return NULL; // out of memory

    struct shp *shp = mem_alloc(sizeof(struct shp));
    shp->address = address;
    shp->refcount = 0;

    hash_set(&shp_table, key, shp);
    return address;
}

void *shm_acquire(const char *key)
{
    struct shp *shp = hash_get(&shp_table, key, NULL);
    if (shp == NULL)
	return NULL; // shp not registered

    shp->refcount++;
    return shp->address;
}

void shm_release(const char *key)
{
    struct shp *shp = hash_get(&shp_table, key, NULL);
    if (shp == NULL)
	return; // shp not registered

    shp->refcount--;
    if (shp->refcount == 0) {
	// no more refs, free the page now
	mem_free(shp, PAGE_SIZE);
    }
}