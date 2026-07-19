#include "common/errors.h"
#include "common/logging.h"
#include "common/utils.h"
#include "ipc/message_queue.h"
#include "process/process.h"
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MQ_NAME "/syscore_mq_prod_cons"
#define NUM_ITEMS 5

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting Producer-Consumer Message Queue Demo");

  syscore_mq_unlink(MQ_NAME);

  syscore_mq_attr_t attr = {
      .flags = 0,
      .max_msgs = 3, // Bounded queue size
      .msg_size = 32,
      .cur_msgs = 0};

  syscore_mq_handle_t mq;
  syscore_error_t err = syscore_mq_open(MQ_NAME, 2, 0, 0666, &attr, &mq);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Failed to open queue: %s", syscore_error_string(err));
    return 1;
  }

  syscore_pid_t pid;
  err = syscore_process_fork(&pid);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Fork failed: %s", syscore_error_string(err));
    syscore_mq_close(mq);
    syscore_mq_unlink(MQ_NAME);
    return 1;
  }

  if (pid == 0) {
    // Producer process
    char item_buf[32];
    for (int i = 0; i < NUM_ITEMS; i++) {
      snprintf(item_buf, sizeof(item_buf), "Item %d", i + 1);
      SYSCORE_LOG_INFO("[Producer] Producing item: %s", item_buf);
      syscore_mq_send(mq, item_buf, strlen(item_buf) + 1, 1);
      struct timespec ts = {.tv_sec = 0, .tv_nsec = 100000000};
      nanosleep(&ts, NULL);
    }
    syscore_mq_close(mq);
    return 0;
  } else {
    // Consumer process
    char item_buf[32];
    unsigned int prio = 0;
    size_t read_bytes = 0;

    for (int i = 0; i < NUM_ITEMS; i++) {
      SYSCORE_LOG_INFO("[Consumer] Waiting for item...");
      syscore_mq_receive(mq, item_buf, sizeof(item_buf), &prio, &read_bytes);
      SYSCORE_LOG_INFO("[Consumer] Consumed item: %s", item_buf);
      struct timespec ts = {.tv_sec = 0, .tv_nsec = 150000000};
      nanosleep(&ts, NULL);
    }

    int status = 0;
    syscore_process_wait(pid, &status, 0);

    syscore_mq_close(mq);
    syscore_mq_unlink(MQ_NAME);
    SYSCORE_LOG_INFO(
        "Producer-Consumer Message Queue Demo completed successfully.");
  }

  return 0;
}
