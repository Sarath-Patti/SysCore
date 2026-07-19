#include "common/errors.h"
#include "common/logging.h"
#include "common/utils.h"
#include "ipc/message_queue.h"
#include <stdio.h>
#include <string.h>

#define MQ_NAME "/syscore_mq_prio"

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting Message Queue Priority Demo");

  syscore_mq_unlink(MQ_NAME);

  syscore_mq_attr_t attr = {
      .flags = 0, .max_msgs = 5, .msg_size = 64, .cur_msgs = 0};

  syscore_mq_handle_t mq;
  syscore_error_t err = syscore_mq_open(MQ_NAME, 2, 0, 0666, &attr, &mq);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Failed to open queue: %s", syscore_error_string(err));
    return 1;
  }

  // Send messages in non-priority order
  const char *msg1 = "Low Priority Message";
  const char *msg2 = "High Priority Message";
  const char *msg3 = "Medium Priority Message";

  SYSCORE_LOG_INFO("Sending message 1 (prio: 5)...");
  syscore_mq_send(mq, msg1, strlen(msg1) + 1, 5);

  SYSCORE_LOG_INFO("Sending message 2 (prio: 20)...");
  syscore_mq_send(mq, msg2, strlen(msg2) + 1, 20);

  SYSCORE_LOG_INFO("Sending message 3 (prio: 10)...");
  syscore_mq_send(mq, msg3, strlen(msg3) + 1, 10);

  // Retrieve messages and verify order
  char buffer[64];
  unsigned int prio = 0;
  size_t read_bytes = 0;

  for (int i = 0; i < 3; i++) {
    err = syscore_mq_receive(mq, buffer, sizeof(buffer), &prio, &read_bytes);
    if (err != SYSCORE_SUCCESS) {
      SYSCORE_LOG_ERROR("Receive failed: %s", syscore_error_string(err));
      syscore_mq_close(mq);
      syscore_mq_unlink(MQ_NAME);
      return 1;
    }
    SYSCORE_LOG_INFO("Received (prio: %u): \"%s\"", prio, buffer);
  }

  syscore_mq_close(mq);
  syscore_mq_unlink(MQ_NAME);
  SYSCORE_LOG_INFO("Priority Message Queue Demo completed successfully.");
  return 0;
}
