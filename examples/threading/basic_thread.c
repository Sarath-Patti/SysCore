#include "common/errors.h"
#include "common/logging.h"
#include "common/utils.h"
#include "threading/threading.h"
#include <stdio.h>

static void *thread_func(void *arg) {
  SYSCORE_UNUSED(arg);
  SYSCORE_LOG_INFO("[Thread] Hello from basic thread!");
  syscore_thread_sleep_ms(500);
  SYSCORE_LOG_INFO("[Thread] Thread exiting.");
  return NULL;
}

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting Basic Thread Demo");

  syscore_thread_t thread;
  syscore_error_t err = syscore_thread_create(&thread, NULL, thread_func, NULL);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Failed to create thread: %s", syscore_error_string(err));
    return 1;
  }

  SYSCORE_LOG_INFO("[Main] Thread created, waiting for it to join...");
  err = syscore_thread_join(thread, NULL);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Failed to join thread: %s", syscore_error_string(err));
    return 1;
  }

  SYSCORE_LOG_INFO("Basic Thread Demo completed successfully.");
  return 0;
}
