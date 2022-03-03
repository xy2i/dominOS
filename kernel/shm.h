#ifndef __SHM_H__
#define __SHM_H__

/**
 * Creates a shared memory page of size 4Ko.
 * If the page is allocated and mapped, its virtual address
 * is returned.
 * @param key The page is registered to the kernel with this key.
 * @return NULL for any kind of error (key is NULL, page already exists,
 * out of memory), otherwise return the virtual address of this page.
 */
void *shm_create(const char *key);
/**
 * Get a reference to a shared memory page.
 * If the page is available, it is mapped for this process.
 * @param key The key the page is registered under.
 * @return NULL if the page is not available, otherwise return the virtual
 * address of the page mapped for that process.
 */
void *shm_acquire(const char *key);
/**
 * Notify kernel that the current process wants to release the reference
 * of the given page. If a reference was acquired using shm_acquire, the page
 * is unmapped and the previously returned virtual address is no longer valid
 * Otherwise, this call has no effect.
 *
 * If this call releases the last reference to a page, the corresponding
 * physical page is freed.
 * @param key The key the page is registered under.
 */
void shm_release(const char *key);

#endif //__SHM_H__
