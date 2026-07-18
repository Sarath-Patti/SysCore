#include "sync/semaphore.h"
#include "common/errors.h"
#include "common/logging.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static syscore_error_t map_sem_error(int err_num) {
  switch (err_num) {
  case 0:
    return SYSCORE_SUCCESS;
  case EACCES:
    return SYSCORE_ERROR_PERMISSION_DENIED;
  case EEXIST:
    return SYSCORE_ERROR_BUSY;
  case EINTR:
    return SYSCORE_ERROR_BUSY;
  case EINVAL:
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  case EMFILE:
    return SYSCORE_ERROR_BUSY;
  case ENAMETOOLONG:
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  case ENFILE:
    return SYSCORE_ERROR_BUSY;
  case ENOENT:
    return SYSCORE_ERROR_NOT_FOUND;
  case ENOMEM:
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  case EOVERFLOW:
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  case EAGAIN:
    return SYSCORE_ERROR_BUSY;
  default:
    return SYSCORE_ERROR_GENERIC;
  }
}

syscore_error_t syscore_sem_init(syscore_sem_t *sem, int shared,
                                 unsigned int value) {
  if (sem == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  SYSCORE_LOG_DEBUG(
      "Initializing unnamed semaphore (shared: %d, value: %u)...", shared,
      value);
  sem->is_named = 0;

#if defined(SYSCORE_OS_MACOS)
  snprintf(sem->macos_name, sizeof(sem->macos_name), "/sc_sem_%d_%p",
           (int)getpid(), (void *)sem);

  // Unlink any potential stale named semaphore first
  sem_unlink(sem->macos_name);

  sem_t *p = sem_open(sem->macos_name, O_CREAT | O_EXCL, 0666, value);
  if (p == SEM_FAILED) {
    int err = errno;
    SYSCORE_LOG_ERROR("sem_open(emulated unnamed) failed for %s: %s (errno %d)",
                      sem->macos_name, strerror(err), err);
    return map_sem_error(err);
  }
  sem->ptr = p;
#else
  sem->ptr = &sem->unnamed;
  if (sem_init(sem->ptr, shared, value) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("sem_init failed: %s (errno %d)", strerror(err), err);
    return map_sem_error(err);
  }
#endif

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_sem_destroy(syscore_sem_t *sem) {
  if (sem == NULL || sem->is_named || sem->ptr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

#if defined(SYSCORE_OS_MACOS)
  SYSCORE_LOG_DEBUG("Destroying emulated unnamed semaphore: %s...",
                    sem->macos_name);
  if (sem_close(sem->ptr) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("sem_close failed on emulated unnamed semaphore: %s "
                      "(errno %d)",
                      strerror(err), err);
    return map_sem_error(err);
  }
  if (sem_unlink(sem->macos_name) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("sem_unlink failed on emulated unnamed semaphore: %s "
                      "(errno %d)",
                      strerror(err), err);
    return map_sem_error(err);
  }
#else
  SYSCORE_LOG_DEBUG("Destroying unnamed semaphore...");
  if (sem_destroy(sem->ptr) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("sem_destroy failed: %s (errno %d)", strerror(err), err);
    return map_sem_error(err);
  }
#endif

  sem->ptr = NULL;
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_sem_open(const char *name, int mode, unsigned int value,
                                 syscore_sem_t *sem) {
  if (name == NULL || sem == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  SYSCORE_LOG_DEBUG("Opening named semaphore: %s (mode: 0%o, value: %u)...",
                    name, mode, value);
  sem->is_named = 1;

  sem_t *p = sem_open(name, O_CREAT, (mode_t)mode, value);
  if (p == SEM_FAILED) {
    int err = errno;
    SYSCORE_LOG_ERROR("sem_open failed for %s: %s (errno %d)", name,
                      strerror(err), err);
    return map_sem_error(err);
  }

  sem->ptr = p;
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_sem_close(syscore_sem_t *sem) {
  if (sem == NULL || !sem->is_named || sem->ptr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  SYSCORE_LOG_DEBUG("Closing named semaphore...");
  if (sem_close(sem->ptr) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("sem_close failed: %s (errno %d)", strerror(err), err);
    return map_sem_error(err);
  }

  sem->ptr = NULL;
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_sem_unlink(const char *name) {
  if (name == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  SYSCORE_LOG_DEBUG("Unlinking named semaphore: %s", name);
  if (sem_unlink(name) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("sem_unlink failed for %s: %s (errno %d)", name,
                      strerror(err), err);
    return map_sem_error(err);
  }

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_sem_wait(syscore_sem_t *sem) {
  if (sem == NULL || sem->ptr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  if (sem_wait(sem->ptr) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("sem_wait failed: %s (errno %d)", strerror(err), err);
    return map_sem_error(err);
  }

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_sem_trywait(syscore_sem_t *sem) {
  if (sem == NULL || sem->ptr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  if (sem_trywait(sem->ptr) < 0) {
    int err = errno;
    if (err != EAGAIN) {
      SYSCORE_LOG_ERROR("sem_trywait failed: %s (errno %d)", strerror(err),
                        err);
    }
    return map_sem_error(err);
  }

  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_sem_post(syscore_sem_t *sem) {
  if (sem == NULL || sem->ptr == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  if (sem_post(sem->ptr) < 0) {
    int err = errno;
    SYSCORE_LOG_ERROR("sem_post failed: %s (errno %d)", strerror(err), err);
    return map_sem_error(err);
  }

  return SYSCORE_SUCCESS;
}
