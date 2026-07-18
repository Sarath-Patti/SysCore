#include "memory/shared_memory.h"
#include "common/errors.h"
#include "common/logging.h"
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static syscore_error_t map_errno(int err_num) {
  switch (err_num) {
  case 0:
    return SYSCORE_SUCCESS;
  case EACCES:
    return SYSCORE_ERROR_PERMISSION_DENIED;
  case EEXIST:
    return SYSCORE_ERROR_BUSY;
  case EINVAL:
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  case EMFILE:
    return SYSCORE_ERROR_BUSY;
  case ENFILE:
    return SYSCORE_ERROR_BUSY;
  case ENOENT:
    return SYSCORE_ERROR_NOT_FOUND;
  case ENOMEM:
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  case EPERM:
    return SYSCORE_ERROR_PERMISSION_DENIED;
  default:
    return SYSCORE_ERROR_GENERIC;
  }
}

syscore_error_t syscore_shm_create(const char *name, size_t size, int mode,
                                   syscore_shm_handle_t *out_handle) {
  if (name == NULL || size == 0 || out_handle == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  SYSCORE_LOG_DEBUG("Creating shared memory object: %s (size: %zu)...", name,
                    size);
  int fd = shm_open(name, O_CREAT | O_RDWR | O_EXCL, (mode_t)mode);
  if (fd < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("shm_open(create) failed for %s: %s (errno %d)", name,
                      strerror(err), err);
    return map_errno(err);
  }

  if (ftruncate(fd, (off_t)size) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("ftruncate failed: %s (errno %d)", strerror(err), err);
    close(fd);
    shm_unlink(name);
    return map_errno(err);
  }

  *out_handle = fd;
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_shm_open(const char *name, int write_mode,
                                 syscore_shm_handle_t *out_handle) {
  if (name == NULL || out_handle == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  int flags = write_mode ? O_RDWR : O_RDONLY;
  SYSCORE_LOG_DEBUG("Opening shared memory object: %s (write_mode: %d)...",
                    name, write_mode);

  int fd = shm_open(name, flags, 0);
  if (fd < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("shm_open failed for %s: %s (errno %d)", name,
                      strerror(err), err);
    return map_errno(err);
  }

  *out_handle = fd;
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_shm_map(syscore_shm_handle_t handle, size_t size,
                                int write_mode, void **out_addr) {
  if (handle < 0 || size == 0 || out_addr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  int prot = PROT_READ;
  if (write_mode) {
    prot |= PROT_WRITE;
  }

  SYSCORE_LOG_DEBUG("Mapping shared memory handle %d (size: %zu)...", handle,
                    size);
  void *addr = mmap(NULL, size, prot, MAP_SHARED, handle, 0);
  if (addr == MAP_FAILED) {
    int err = errno;
    SYSCORE_LOG_ERROR("mmap failed: %s (errno %d)", strerror(err), err);
    return map_errno(err);
  }

  *out_addr = addr;
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_shm_unmap(void *addr, size_t size) {
  if (addr == NULL || size == 0) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  SYSCORE_LOG_DEBUG("Unmapping shared memory address %p (size: %zu)...", addr,
                    size);
  if (munmap(addr, size) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("munmap failed: %s (errno %d)", strerror(err), err);
    return map_errno(err);
  }

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_shm_close(syscore_shm_handle_t handle) {
  if (handle < 0) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  SYSCORE_LOG_DEBUG("Closing shared memory handle: %d", handle);
  if (close(handle) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("close failed: %s (errno %d)", strerror(err), err);
    return map_errno(err);
  }

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_shm_destroy(const char *name) {
  if (name == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  SYSCORE_LOG_DEBUG("Unlinking shared memory object: %s", name);
  if (shm_unlink(name) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("shm_unlink failed for %s: %s (errno %d)", name,
                      strerror(err), err);
    return map_errno(err);
  }

  return SYSCORE_SUCCESS;
}
