#include "common/errors.h"
#include "common/logging.h"
#include "common/utils.h"
#include "sync/semaphore.h"
#include "threading/threading.h"
#include <stdio.h>

#define NUM_THREADS 3

static syscore_sem_t g_sem;

static void *worker_func(void *arg) {
  int id = *(int *)arg;
  SYSCORE_LOG_INFO("[Thread %d] Waiting to enter critical section...", id);

  syscore_sem_wait(&g_sem);
  SYSCORE_LOG_INFO("[Thread %d] Entered critical section.", id);

  // Simulate some exclusive work
  syscore_thread_sleep_ms(200);

  SYSCORE_LOG_INFO("[Thread %d] Leaving critical section.", id);
  syscore_sem_post(&g_sem);

  return NULL;
}

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting Unnamed Semaphore Demo (Binary Semaphore)");

  // Initialize semaphore with value 1 (acting as a mutex)
  syscore_error_t err = syscore_sem_init(&g_sem, 0, 1);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Failed to initialize semaphore: %s",
                      syscore_error_string(err));
    return 1;
  }

  syscore_thread_t threads[NUM_THREADS];
  int thread_ids[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; i++) {
    thread_ids[i] = i + 1;
    syscore_thread_create(&threads[i], NULL, worker_func, &thread_ids[i]);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    syscore_thread_join(threads[i], NULL);
  }

  syscore_sem_destroy(&g_sem);
  SYSCORE_LOG_INFO("Unnamed Semaphore Demo completed successfully.");
  return 0;
}
