#ifndef SYSCORE_SYNC_H
#define SYSCORE_SYNC_H

#include "common/errors.h"
#include <pthread.h>

/**
 * Structural wrapper for pthread_mutex_t.
 */
typedef struct {
  pthread_mutex_t raw;
} syscore_mutex_t;

/**
 * Structural wrapper for pthread_cond_t.
 */
typedef struct {
  pthread_cond_t raw;
} syscore_cond_t;

/**
 * Structural wrapper for pthread_rwlock_t.
 */
typedef struct {
  pthread_rwlock_t raw;
} syscore_rwlock_t;

/* Mutex wrappers */
syscore_error_t syscore_mutex_init(syscore_mutex_t *mutex);
syscore_error_t syscore_mutex_lock(syscore_mutex_t *mutex);
syscore_error_t syscore_mutex_unlock(syscore_mutex_t *mutex);
syscore_error_t syscore_mutex_destroy(syscore_mutex_t *mutex);

/* Condition Variable wrappers */
syscore_error_t syscore_cond_init(syscore_cond_t *cond);
syscore_error_t syscore_cond_wait(syscore_cond_t *cond, syscore_mutex_t *mutex);
syscore_error_t syscore_cond_signal(syscore_cond_t *cond);
syscore_error_t syscore_cond_broadcast(syscore_cond_t *cond);
syscore_error_t syscore_cond_destroy(syscore_cond_t *cond);

/* Read-Write Lock wrappers */
syscore_error_t syscore_rwlock_init(syscore_rwlock_t *rwlock);
syscore_error_t syscore_rwlock_rdlock(syscore_rwlock_t *rwlock);
syscore_error_t syscore_rwlock_wrlock(syscore_rwlock_t *rwlock);
syscore_error_t syscore_rwlock_unlock(syscore_rwlock_t *rwlock);
syscore_error_t syscore_rwlock_destroy(syscore_rwlock_t *rwlock);

#endif // SYSCORE_SYNC_H
