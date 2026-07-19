#ifndef SYSCORE_MMAP_H
#define SYSCORE_MMAP_H

#include "common/errors.h"
#include <stddef.h>
#include <sys/types.h>

// Protection flags
#define SYSCORE_PROT_NONE 0x0
#define SYSCORE_PROT_READ 0x1
#define SYSCORE_PROT_WRITE 0x2
#define SYSCORE_PROT_EXEC 0x4

// Mapping flags
#define SYSCORE_MAP_SHARED 0x01
#define SYSCORE_MAP_PRIVATE 0x02
#define SYSCORE_MAP_ANONYMOUS 0x04

// msync flags
#define SYSCORE_MS_ASYNC 0x01
#define SYSCORE_MS_SYNC 0x02
#define SYSCORE_MS_INVALIDATE 0x04

/**
 * Creates a new mapping in the virtual address space of the calling process.
 *
 * @param addr Starting address for the new mapping (can be NULL for system
 * choice).
 * @param length Length of the mapping in bytes.
 * @param prot Access protection of the mapping (bitwise OR of SYSCORE_PROT_*
 * flags).
 * @param flags Mapping visibility and type (bitwise OR of SYSCORE_MAP_* flags).
 * @param fd File descriptor (use -1 for anonymous mapping).
 * @param offset Offset in the file (use 0 for anonymous mapping).
 * @param out_addr Pointer to store the starting address of the mapped region.
 */
syscore_error_t syscore_mmap(void *addr, size_t length, int prot, int flags,
                             int fd, off_t offset, void **out_addr);

/**
 * Deletes the mappings for the specified address range.
 *
 * @param addr Starting address of the range to unmap.
 * @param length Length of the range in bytes.
 */
syscore_error_t syscore_munmap(void *addr, size_t length);

/**
 * Changes access protections for the calling process's memory pages.
 *
 * @param addr Starting address of the memory pages.
 * @param length Length of the region in bytes.
 * @param prot New access protection (bitwise OR of SYSCORE_PROT_* flags).
 */
syscore_error_t syscore_mprotect(void *addr, size_t length, int prot);

/**
 * Flushes changes made to in-memory copy of a file back to disk.
 *
 * @param addr Starting address of the mapped region.
 * @param length Length of the region in bytes.
 * @param flags Synchronization behavior (bitwise OR of SYSCORE_MS_* flags).
 */
syscore_error_t syscore_msync(void *addr, size_t length, int flags);

#endif // SYSCORE_MMAP_H
