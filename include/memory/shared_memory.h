#ifndef SYSCORE_SHARED_MEMORY_H
#define SYSCORE_SHARED_MEMORY_H

#include "common/errors.h"
#include <stddef.h>

/**
 * Handle type representing a shared memory descriptor (file descriptor).
 */
typedef int syscore_shm_handle_t;

#define SYSCORE_SHM_INVALID_HANDLE (-1)

/**
 * Creates a new POSIX shared memory object.
 *
 * @param name The name of the shared memory object (should start with '/').
 * @param size The size of the shared memory object in bytes.
 * @param mode Permission bits (e.g. 0666).
 * @param out_handle Pointer to store the created shared memory handle.
 */
syscore_error_t syscore_shm_create(const char *name, size_t size, int mode,
                                   syscore_shm_handle_t *out_handle);

/**
 * Opens an existing POSIX shared memory object.
 *
 * @param name The name of the shared memory object.
 * @param write_mode If non-zero, open for reading and writing; otherwise, open
 * for reading only.
 * @param out_handle Pointer to store the opened shared memory handle.
 */
syscore_error_t syscore_shm_open(const char *name, int write_mode,
                                 syscore_shm_handle_t *out_handle);

/**
 * Maps the shared memory object into the calling process's address space.
 *
 * @param handle The shared memory handle.
 * @param size The size of the shared memory object.
 * @param write_mode If non-zero, map with read-write permissions; otherwise,
 * map with read-only permissions.
 * @param out_addr Pointer to store the mapped starting address.
 */
syscore_error_t syscore_shm_map(syscore_shm_handle_t handle, size_t size,
                                int write_mode, void **out_addr);

/**
 * Unmaps the shared memory from the process's address space.
 *
 * @param addr The starting address of the mapped region.
 * @param size The size of the mapped region.
 */
syscore_error_t syscore_shm_unmap(void *addr, size_t size);

/**
 * Closes the shared memory descriptor.
 *
 * @param handle The shared memory handle.
 */
syscore_error_t syscore_shm_close(syscore_shm_handle_t handle);

/**
 * Destroys/unlinks a POSIX shared memory object name.
 *
 * @param name The name of the shared memory object.
 */
syscore_error_t syscore_shm_destroy(const char *name);

#endif // SYSCORE_SHARED_MEMORY_H
