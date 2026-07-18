#include "common/errors.h"
#include "common/logging.h"
#include "threading/threading.h"
#include <stdio.h>

typedef struct {
  int id;
  const char *message;
} thread_data_t;

static void *thread_func(void *arg) {
  if (arg == NULL) {
    SYSCORE_LOG_ERROR("[Thread] Error: Thread arguments are NULL!");
    return NULL;
  }

  thread_data_t *data = (thread_data_t *)arg;
  SYSCORE_LOG_INFO("[Thread %d] Message: %s", data->id, data->message);
  syscore_thread_sleep_ms(300);
  SYSCORE_LOG_INFO("[Thread %d] Exiting.", data->id);
  return NULL;
}

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting Thread Arguments Demo");

  thread_data_t data = {.id = 42,
                        .message = "Hello from main process via pointer args!"};

  syscore_thread_t thread;
  syscore_error_t err = syscore_thread_create(&thread, NULL, thread_func, &data);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Failed to create thread: %s", syscore_error_string(err));
    return 1;
  }

  SYSCORE_LOG_INFO("[Main] Thread created with arguments. Waiting to join...");
  err = syscore_thread_join(thread, NULL);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Failed to join thread: %s", syscore_error_string(err));
    return 1;
  }

  SYSCORE_LOG_INFO("Thread Arguments Demo completed successfully.");
  return 0;
}
