#ifndef __SHM_H__
#define __SHM_H__

/**
 * Init shared memory.
 */
void shm_init();

/* see primitive.h for doc */
void *shm_create(const char *key);
void *shm_acquire(const char *key);
void shm_release(const char *key);

#endif //__SHM_H__
