#include "common/errors.h"
#include "ipc/message_queue.h"
#include "test_helpers.h"
#include <string.h>

#define MQ_TEST_NAME "/sc_mq_test"

int main(void) {
  syscore_mq_unlink(MQ_TEST_NAME);

  syscore_mq_attr_t attr = {
      .flags = 0, .max_msgs = 5, .msg_size = 64, .cur_msgs = 0};

  syscore_mq_handle_t mq;
  syscore_error_t err =
      syscore_mq_open(MQ_TEST_NAME, 2, 0, 0666, &attr, &mq); // read-write
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  const char *msg = "SysCoreMqTest";
  err = syscore_mq_send(mq, msg, strlen(msg) + 1, 10);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  char buffer[64];
  unsigned int prio = 0;
  size_t read_bytes = 0;
  err = syscore_mq_receive(mq, buffer, sizeof(buffer), &prio, &read_bytes);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);
  ASSERT_INT_EQ(prio, 10);
  ASSERT_TRUE(strcmp(buffer, msg) == 0);

  syscore_mq_close(mq);
  syscore_mq_unlink(MQ_TEST_NAME);

  printf("test_mq passed.\n");
  return 0;
}
