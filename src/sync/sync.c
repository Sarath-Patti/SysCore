#include "sync/sync.h"
#include "common/errors.h"
#include "common/logging.h"
#include <errno.h>
#include <string.h>

static syscore_error_t map_pthread_error(int err_num) {
  switch (err_num) {
  case 0:
    return SYSCORE_SUCCESS;
  case EAGAIN:
    return SYSCORE_ERROR_BUSY;
  case EINVAL:
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  case EPERM:
    return SYSCORE_ERROR_PERMISSION_DENIED;
  case EDEADLK:
    return SYSCORE_ERROR_BUSY;
  case EBUSY:
    return SYSCORE_ERROR_BUSY;
  case ENOMEM:
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  default:
    return SYSCORE_ERROR_GENERIC;
  }
}

/* Mutex wrappers implementation */
syscore_error_t syscore_mutex_init(syscore_mutex_t *mutex) {
  if (mutex == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  SYSCORE_LOG_DEBUG("Initializing mutex...");
  int rc = pthread_mutex_init(&mutex->raw, NULL);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_mutex_init failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mutex_lock(syscore_mutex_t *mutex) {
  if (mutex == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  int rc = pthread_mutex_lock(&mutex->raw);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_mutex_lock failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mutex_unlock(syscore_mutex_t *mutex) {
  if (mutex == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  int rc = pthread_mutex_unlock(&mutex->raw);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_mutex_unlock failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_mutex_destroy(syscore_mutex_t *mutex) {
  if (mutex == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  SYSCORE_LOG_DEBUG("Destroying mutex...");
  int rc = pthread_mutex_destroy(&mutex->raw);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_mutex_destroy failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}

/* Condition Variable wrappers implementation */
syscore_error_t syscore_cond_init(syscore_cond_t *cond) {
  if (cond == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  SYSCORE_LOG_DEBUG("Initializing condition variable...");
  int rc = pthread_cond_init(&cond->raw, NULL);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_cond_init failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_cond_wait(syscore_cond_t *cond,
                                  syscore_mutex_t *mutex) {
  if (cond == NULL || mutex == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  int rc = pthread_cond_wait(&cond->raw, &mutex->raw);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_cond_wait failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_cond_signal(syscore_cond_t *cond) {
  if (cond == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  int rc = pthread_cond_signal(&cond->raw);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_cond_signal failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_cond_broadcast(syscore_cond_t *cond) {
  if (cond == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  int rc = pthread_cond_broadcast(&cond->raw);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_cond_broadcast failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_cond_destroy(syscore_cond_t *cond) {
  if (cond == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  SYSCORE_LOG_DEBUG("Destroying condition variable...");
  int rc = pthread_cond_destroy(&cond->raw);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_cond_destroy failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}

/* Read-Write Lock wrappers implementation */
syscore_error_t syscore_rwlock_init(syscore_rwlock_t *rwlock) {
  if (rwlock == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  SYSCORE_LOG_DEBUG("Initializing read-write lock...");
  int rc = pthread_rwlock_init(&rwlock->raw, NULL);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_rwlock_init failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_rwlock_rdlock(syscore_rwlock_t *rwlock) {
  if (rwlock == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  int rc = pthread_rwlock_rdlock(&rwlock->raw);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_rwlock_rdlock failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_rwlock_wrlock(syscore_rwlock_t *rwlock) {
  if (rwlock == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  int rc = pthread_rwlock_wrlock(&rwlock->raw);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_rwlock_wrlock failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_rwlock_unlock(syscore_rwlock_t *rwlock) {
  if (rwlock == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  int rc = pthread_rwlock_unlock(&rwlock->raw);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_rwlock_unlock failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}

syscore_error_t syscore_rwlock_destroy(syscore_rwlock_t *rwlock) {
  if (rwlock == NULL) {
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }
  SYSCORE_LOG_DEBUG("Destroying read-write lock...");
  int rc = pthread_rwlock_destroy(&rwlock->raw);
  if (rc != 0) {
    SYSCORE_LOG_ERROR("pthread_rwlock_destroy failed: %s", strerror(rc));
    return map_pthread_error(rc);
  }
  return SYSCORE_SUCCESS;
}
