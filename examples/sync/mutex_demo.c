#include "common/errors.h"
#include "common/logging.h"
#include "common/utils.h"
#include "sync/sync.h"
#include "threading/threading.h"
#include <stdio.h>

#define NUM_THREADS 4
#define ITERATIONS 10000

static int g_counter = 0;
static syscore_mutex_t g_mutex;

static void *thread_func(void *arg) {
  SYSCORE_UNUSED(arg);
  for (int i = 0; i < ITERATIONS; i++) {
    syscore_mutex_lock(&g_mutex);
    g_counter++;
    syscore_mutex_unlock(&g_mutex);
  }
  return NULL;
}

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting Mutex Counter Demo");

  syscore_error_t err = syscore_mutex_init(&g_mutex);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Failed to initialize mutex: %s",
                      syscore_error_string(err));
    return 1;
  }

  syscore_thread_t threads[NUM_THREADS];
  for (int i = 0; i < NUM_THREADS; i++) {
    err = syscore_thread_create(&threads[i], NULL, thread_func, NULL);
    if (err != SYSCORE_SUCCESS) {
      SYSCORE_LOG_ERROR("Failed to create thread %d: %s", i,
                        syscore_error_string(err));
      return 1;
    }
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    syscore_thread_join(threads[i], NULL);
  }

  SYSCORE_LOG_INFO("Final Counter Value: %d (Expected: %d)", g_counter,
                   NUM_THREADS * ITERATIONS);

  syscore_mutex_destroy(&g_mutex);

  if (g_counter == NUM_THREADS * ITERATIONS) {
    SYSCORE_LOG_INFO("Mutex Counter Demo completed successfully.");
    return 0;
  } else {
    SYSCORE_LOG_ERROR("Mutex Counter Demo failed. Counter mismatch.");
    return 1;
  }
}
