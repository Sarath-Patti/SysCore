#include "common/errors.h"
#include "common/logging.h"
#include "common/utils.h"
#include "threading/threading.h"
#include <stdio.h>

#define NUM_THREADS 4

typedef struct {
  int id;
} thread_context_t;

static void *thread_func(void *arg) {
  if (arg == NULL) {
    return NULL;
  }

  thread_context_t *ctx = (thread_context_t *)arg;
  SYSCORE_LOG_INFO("[Thread %d] Started.", ctx->id);

  // Simulate some work
  syscore_thread_sleep_ms(100 * ctx->id);

  SYSCORE_LOG_INFO("[Thread %d] Finished work, exiting.", ctx->id);
  return NULL;
}

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting Multiple Threads Demo");

  syscore_thread_t threads[NUM_THREADS];
  thread_context_t contexts[NUM_THREADS];

  // Spawn threads
  for (int i = 0; i < NUM_THREADS; i++) {
    contexts[i].id = i + 1;
    syscore_error_t err =
        syscore_thread_create(&threads[i], NULL, thread_func, &contexts[i]);
    if (err != SYSCORE_SUCCESS) {
      SYSCORE_LOG_ERROR("Failed to spawn thread %d: %s", i + 1,
                        syscore_error_string(err));
      return 1;
    }
  }

  SYSCORE_LOG_INFO("[Main] All threads spawned. Joining them...");

  // Join threads
  for (int i = 0; i < NUM_THREADS; i++) {
    syscore_error_t err = syscore_thread_join(threads[i], NULL);
    if (err != SYSCORE_SUCCESS) {
      SYSCORE_LOG_ERROR("Failed to join thread %d: %s", i + 1,
                        syscore_error_string(err));
      return 1;
    }
  }

  SYSCORE_LOG_INFO("Multiple Threads Demo completed successfully.");
  return 0;
}
