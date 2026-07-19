#include "memory/mmap.h"
#include "common/errors.h"
#include "common/logging.h"
#include <errno.h>
#include <string.h>
#include <sys/mman.h>

static int map_prot(int syscore_prot) {
  int prot = PROT_NONE;
  if (syscore_prot & SYSCORE_PROT_READ) {
    prot |= PROT_READ;
  }
  if (syscore_prot & SYSCORE_PROT_WRITE) {
    prot |= PROT_WRITE;
  }
  if (syscore_prot & SYSCORE_PROT_EXEC) {
    prot |= PROT_EXEC;
  }
  return prot;
}

static int map_flags(int syscore_flags) {
  int flags = 0;
  if (syscore_flags & SYSCORE_MAP_SHARED) {
    flags |= MAP_SHARED;
  }
  if (syscore_flags & SYSCORE_MAP_PRIVATE) {
    flags |= MAP_PRIVATE;
  }
  if (syscore_flags & SYSCORE_MAP_ANONYMOUS) {
#if defined(MAP_ANONYMOUS)
    flags |= MAP_ANONYMOUS;
#else
    flags |= MAP_ANON;
#endif
  }
  return flags;
}

static int map_msync_flags(int syscore_msync_flags) {
  int flags = 0;
  if (syscore_msync_flags & SYSCORE_MS_ASYNC) {
    flags |= MS_ASYNC;
  }
  if (syscore_msync_flags & SYSCORE_MS_SYNC) {
    flags |= MS_SYNC;
  }
  if (syscore_msync_flags & SYSCORE_MS_INVALIDATE) {
    flags |= MS_INVALIDATE;
  }
  return flags;
}

static syscore_error_t map_mmap_error(int err_num) {
  switch (err_num) {
  case 0:
    return SYSCORE_SUCCESS;
  case EACCES:
    return SYSCORE_ERROR_PERMISSION_DENIED;
  case EAGAIN:
    return SYSCORE_ERROR_BUSY;
  case EBADF:
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  case EINVAL:
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  case ENFILE:
  case EMFILE:
    return SYSCORE_ERROR_BUSY;
  case ENODEV:
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  case ENOMEM:
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  case EPERM:
    return SYSCORE_ERROR_PERMISSION_DENIED;
  default:
    return SYSCORE_ERROR_GENERIC;
  }
}

syscore_error_t syscore_mmap(void *addr, size_t length, int prot, int flags,
                             int fd, off_t offset, void **out_addr) {
  if (out_addr == NULL || length == 0) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  int posix_prot = map_prot(prot);
  int posix_flags = map_flags(flags);

  SYSCORE_LOG_DEBUG(
      "Calling mmap(addr: %p, length: %zu, prot: 0x%x, flags: 0x%x)...", addr,
      length, posix_prot, posix_flags);

  void *mapped = mmap(addr, length, posix_prot, posix_flags, fd, offset);
  if (mapped == MAP_FAILED) {
    int err = errno;
    SYSCORE_LOG_ERROR("mmap failed: %s (errno %d)", strerror(err), err);
    return map_mmap_error(err);
  }

  *out_addr = mapped;
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_munmap(void *addr, size_t length) {
  if (addr == NULL || length == 0) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  SYSCORE_LOG_DEBUG("Calling munmap(addr: %p, length: %zu)...", addr, length);

  if (munmap(addr, length) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("munmap failed: %s (errno %d)", strerror(err), err);
    return map_mmap_error(err);
  }

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mprotect(void *addr, size_t length, int prot) {
  if (addr == NULL || length == 0) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  int posix_prot = map_prot(prot);
  SYSCORE_LOG_DEBUG("Calling mprotect(addr: %p, length: %zu, prot: 0x%x)...",
                    addr, length, posix_prot);

  if (mprotect(addr, length, posix_prot) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("mprotect failed: %s (errno %d)", strerror(err), err);
    return map_mmap_error(err);
  }

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_msync(void *addr, size_t length, int flags) {
  if (addr == NULL || length == 0) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  int posix_flags = map_msync_flags(flags);
  SYSCORE_LOG_DEBUG("Calling msync(addr: %p, length: %zu, flags: 0x%x)...",
                    addr, length, posix_flags);

  if (msync(addr, length, posix_flags) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("msync failed: %s (errno %d)", strerror(err), err);
    return map_mmap_error(err);
  }

  return SYSCORE_SUCCESS;
}
