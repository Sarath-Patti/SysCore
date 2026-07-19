#include "common/errors.h"
#include "ipc/ipc.h"
#include "test_helpers.h"
#include <string.h>

int main(void) {
  syscore_ipc_handle_t rd, wr;
  syscore_error_t err = syscore_ipc_pipe_create(&rd, &wr);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  const char *msg = "SysCorePipeTest";
  size_t written = 0;
  err = syscore_ipc_write(wr, msg, strlen(msg), &written);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);
  ASSERT_INT_EQ(written, strlen(msg));

  char buffer[32];
  memset(buffer, 0, sizeof(buffer));
  size_t read_bytes = 0;
  err = syscore_ipc_read(rd, buffer, written, &read_bytes);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);
  ASSERT_INT_EQ(read_bytes, written);
  ASSERT_TRUE(strcmp(buffer, msg) == 0);

  syscore_ipc_close(rd);
  syscore_ipc_close(wr);

  printf("test_ipc passed.\n");
  return 0;
}
