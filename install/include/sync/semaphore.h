#ifndef SYSCORE_SEMAPHORE_H
#define SYSCORE_SEMAPHORE_H

#include "common/errors.h"
#include "common/utils.h"
#include <semaphore.h>

/**
 * Handle type representing a SysCore Semaphore, wrapping named or unnamed POSIX
 * semaphores.
 */
typedef struct {
  sem_t *ptr;      // Points to named or raw unnamed semaphore
  sem_t unnamed;   // Storage for unnamed semaphore
  int is_named;    // Flag indicating if it is named
#if defined(SYSCORE_OS_MACOS)
  char macos_name[64]; // Stores unique name for macOS unnamed semaphore
#endif
} syscore_sem_t;


/**
 * Initializes an unnamed/anonymous semaphore (wrapper for sem_init).
 *
 * @param sem Pointer to the semaphore structure.
 * @param shared If non-zero, the semaphore can be shared between processes.
 * @param value Initial value of the semaphore.
 */
syscore_error_t syscore_sem_init(syscore_sem_t *sem, int shared,
                                 unsigned int value);

/**
 * Destroys an unnamed semaphore (wrapper for sem_destroy).
 *
 * @param sem Pointer to the semaphore structure.
 */
syscore_error_t syscore_sem_destroy(syscore_sem_t *sem);

/**
 * Creates or opens a named semaphore (wrapper for sem_open).
 *
 * @param name The name of the semaphore (should start with '/').
 * @param mode Permission bits (e.g. 0666).
 * @param value Initial value of the semaphore.
 * @param sem Pointer to the semaphore structure to populate.
 */
syscore_error_t syscore_sem_open(const char *name, int mode, unsigned int value,
                                 syscore_sem_t *sem);

/**
 * Closes an opened named semaphore (wrapper for sem_close).
 *
 * @param sem Pointer to the semaphore structure.
 */
syscore_error_t syscore_sem_close(syscore_sem_t *sem);

/**
 * Unlinks a named semaphore (wrapper for sem_unlink).
 *
 * @param name The name of the semaphore.
 */
syscore_error_t syscore_sem_unlink(const char *name);

/**
 * Decrements (locks) the semaphore (wrapper for sem_wait).
 *
 * @param sem Pointer to the semaphore structure.
 */
syscore_error_t syscore_sem_wait(syscore_sem_t *sem);

/**
 * Non-blocking decrement of the semaphore (wrapper for sem_trywait).
 *
 * @param sem Pointer to the semaphore structure.
 */
syscore_error_t syscore_sem_trywait(syscore_sem_t *sem);

/**
 * Increments (unlocks) the semaphore (wrapper for sem_post).
 *
 * @param sem Pointer to the semaphore structure.
 */
syscore_error_t syscore_sem_post(syscore_sem_t *sem);

#endif // SYSCORE_SEMAPHORE_H
