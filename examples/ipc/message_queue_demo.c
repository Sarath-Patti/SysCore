#include "common/errors.h"
#include "common/logging.h"
#include "common/utils.h"
#include "ipc/message_queue.h"
#include "process/process.h"
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MQ_NAME "/syscore_mq_demo"

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting POSIX Message Queue Basic Demo");

  syscore_mq_unlink(MQ_NAME);

  syscore_mq_attr_t attr = {
      .flags = 0, .max_msgs = 5, .msg_size = 128, .cur_msgs = 0};

  syscore_mq_handle_t mq;
  syscore_error_t err =
      syscore_mq_open(MQ_NAME, 2, 0, 0666, &attr, &mq); // read-write
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Failed to open message queue: %s",
                      syscore_error_string(err));
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
    // Child process sends message
    const char *msg = "Hello from Child process via Message Queue!";
    SYSCORE_LOG_INFO("[Child] Sending message to queue...");
    err = syscore_mq_send(mq, msg, strlen(msg) + 1, 1);
    if (err != SYSCORE_SUCCESS) {
      SYSCORE_LOG_ERROR("[Child] Send failed: %s", syscore_error_string(err));
      syscore_mq_close(mq);
      return 1;
    }
    SYSCORE_LOG_INFO("[Child] Message sent successfully.");
    syscore_mq_close(mq);
    return 0;
  } else {
    // Parent process receives message
    char buffer[128];
    unsigned int prio = 0;
    size_t read_bytes = 0;

    // Sleep briefly to let child send first
    usleep(100000);

    SYSCORE_LOG_INFO("[Parent] Receiving message from queue...");
    err = syscore_mq_receive(mq, buffer, sizeof(buffer), &prio, &read_bytes);
    if (err != SYSCORE_SUCCESS) {
      SYSCORE_LOG_ERROR("[Parent] Receive failed: %s",
                        syscore_error_string(err));
      syscore_mq_close(mq);
      syscore_mq_unlink(MQ_NAME);
      return 1;
    }

    SYSCORE_LOG_INFO("[Parent] Received message (size: %zu, prio: %u): \"%s\"",
                     read_bytes, prio, buffer);

    // Wait for child
    int status = 0;
    syscore_process_wait(pid, &status, 0);

    syscore_mq_close(mq);
    syscore_mq_unlink(MQ_NAME);
    SYSCORE_LOG_INFO("Basic Message Queue Demo completed successfully.");
  }

  return 0;
}
