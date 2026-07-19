#include "common/errors.h"
#include "process/process.h"
#include "test_helpers.h"
#include <unistd.h>

int main(void) {
  syscore_pid_t pid = syscore_process_get_pid();
  ASSERT_TRUE(pid > 0);

  syscore_pid_t ppid = syscore_process_get_ppid();
  ASSERT_TRUE(ppid > 0);

  syscore_pid_t child_pid;
  syscore_error_t err = syscore_process_fork(&child_pid);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  if (child_pid == 0) {
    // Child exit quickly
    _exit(42);
  } else {
    int status = 0;
    err = syscore_process_wait(child_pid, &status, 0);
    ASSERT_INT_EQ(err, SYSCORE_SUCCESS);
    ASSERT_TRUE(WIFEXITED(status));
    ASSERT_INT_EQ(WEXITSTATUS(status), 42);
  }

  printf("test_process passed.\n");
  return 0;
}
