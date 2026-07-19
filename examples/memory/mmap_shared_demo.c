#include "common/errors.h"
#include "common/logging.h"
#include "memory/mmap.h"
#include "process/process.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define FILE_NAME "mmap_shared_temp.txt"
#define FILE_SIZE 64

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting mmap Shared Process Mapping Demo");

  int fd = open(FILE_NAME, O_RDWR | O_CREAT | O_TRUNC, 0666);
  if (fd < 0) {
    SYSCORE_LOG_ERROR("Failed to open file: %s", FILE_NAME);
    return 1;
  }

  if (ftruncate(fd, FILE_SIZE) < 0) {
    SYSCORE_LOG_ERROR("Failed to set file size");
    close(fd);
    unlink(FILE_NAME);
    return 1;
  }

  void *addr = NULL;
  syscore_error_t err = syscore_mmap(
      NULL, FILE_SIZE, SYSCORE_PROT_READ | SYSCORE_PROT_WRITE,
      SYSCORE_MAP_SHARED, fd, 0, &addr);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("mmap shared failed: %s", syscore_error_string(err));
    close(fd);
    unlink(FILE_NAME);
    return 1;
  }

  strcpy((char *)addr, "Initial Parent String");

  syscore_pid_t pid;
  err = syscore_process_fork(&pid);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Fork failed: %s", syscore_error_string(err));
    syscore_munmap(addr, FILE_SIZE);
    close(fd);
    unlink(FILE_NAME);
    return 1;
  }

  if (pid == 0) {
    // Child process updates shared memory
    SYSCORE_LOG_INFO("[Child] Mapped content initially: \"%s\"", (char *)addr);
    SYSCORE_LOG_INFO("[Child] Updating mapped content...");
    strcpy((char *)addr, "Updated String from Child!");
    syscore_munmap(addr, FILE_SIZE);
    close(fd);
    return 0;
  } else {
    // Parent process waits, then reads updates
    int status = 0;
    syscore_process_wait(pid, &status, 0);

    SYSCORE_LOG_INFO("[Parent] Mapped content after child exit: \"%s\"",
                     (char *)addr);

    syscore_munmap(addr, FILE_SIZE);
    close(fd);
    unlink(FILE_NAME);
    SYSCORE_LOG_INFO("Shared process mapping demo finished successfully.");
  }

  return 0;
}
