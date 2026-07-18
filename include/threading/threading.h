#ifndef SYSCORE_THREADING_H
#define SYSCORE_THREADING_H

#include "common/errors.h"
#include <pthread.h>
#include <stddef.h>

/**
 * Handle type for a SysCore thread, wrapping POSIX pthread_t.
 */
typedef pthread_t syscore_thread_t;

/**
 * Thread attributes structure.
 */
typedef struct {
  size_t stack_size;
  int detach_state; // 0 for joinable, 1 for detached
} syscore_thread_attr_t;

/**
 * Type signature for the thread entry function.
 */
typedef void *(*syscore_thread_func_t)(void *);

/**
 * Initializes thread attributes to default values.
 *
 * @param attr Pointer to attributes structure to initialize.
 */
syscore_error_t syscore_thread_attr_init(syscore_thread_attr_t *attr);

/**
 * Spawns a new thread executing the given start routine.
 *
 * @param out_thread Pointer to store the created thread handle.
 * @param attr Thread attributes (can be NULL for defaults).
 * @param func The function to execute in the new thread.
 * @param arg Context argument passed to the function.
 */
syscore_error_t syscore_thread_create(syscore_thread_t *out_thread,
                                      const syscore_thread_attr_t *attr,
                                      syscore_thread_func_t func, void *arg);

/**
 * Waits for the specified thread to terminate.
 *
 * @param thread The thread handle to wait for.
 * @param out_retval Pointer to store the return value of the thread function
 * (can be NULL).
 */
syscore_error_t syscore_thread_join(syscore_thread_t thread, void **out_retval);

/**
 * Retrieves the handle of the calling thread.
 */
syscore_thread_t syscore_thread_self(void);

/**
 * Puts the calling thread to sleep for the specified duration in milliseconds.
 *
 * @param ms Sleep duration in milliseconds.
 */
void syscore_thread_sleep_ms(unsigned int ms);

#endif // SYSCORE_THREADING_H
