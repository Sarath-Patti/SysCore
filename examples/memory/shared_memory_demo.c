#include "common/errors.h"
#include "common/logging.h"
#include "common/utils.h"
#include "memory/shared_memory.h"
#include "process/process.h"
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SHM_NAME "/syscore_shm_demo"
#define SHM_SIZE 256
#define SHM_MSG "Hello from Parent via Shared Memory!"

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting Shared Memory Demo");

  // Clean up any stale SHM object
  syscore_shm_destroy(SHM_NAME);

  syscore_shm_handle_t shm_fd;
  syscore_error_t err = syscore_shm_create(SHM_NAME, SHM_SIZE, 0666, &shm_fd);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Failed to create shared memory: %s",
                      syscore_error_string(err));
    return 1;
  }

  syscore_pid_t pid;
  err = syscore_process_fork(&pid);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Fork failed: %s", syscore_error_string(err));
    syscore_shm_close(shm_fd);
    syscore_shm_destroy(SHM_NAME);
    return 1;
  }

  if (pid == 0) {
    // Child process reads
    syscore_shm_handle_t child_fd;
    err = syscore_shm_open(SHM_NAME, 0, &child_fd); // Read-only
    if (err != SYSCORE_SUCCESS) {
      SYSCORE_LOG_ERROR("[Child] Open failed: %s", syscore_error_string(err));
      return 1;
    }

    void *addr = NULL;
    err = syscore_shm_map(child_fd, SHM_SIZE, 0, &addr);
    if (err != SYSCORE_SUCCESS) {
      SYSCORE_LOG_ERROR("[Child] Map failed: %s", syscore_error_string(err));
      syscore_shm_close(child_fd);
      return 1;
    }

    // Wait a bit for parent to write
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 100000000};
    nanosleep(&ts, NULL);

    SYSCORE_LOG_INFO("[Child] Read message from shared memory: \"%s\"",
                     (char *)addr);

    syscore_shm_unmap(addr, SHM_SIZE);
    syscore_shm_close(child_fd);
    return 0;
  } else {
    // Parent process writes
    void *addr = NULL;
    err = syscore_shm_map(shm_fd, SHM_SIZE, 1, &addr); // Read-write
    if (err != SYSCORE_SUCCESS) {
      SYSCORE_LOG_ERROR("[Parent] Map failed: %s", syscore_error_string(err));
      syscore_shm_close(shm_fd);
      syscore_shm_destroy(SHM_NAME);
      return 1;
    }

    SYSCORE_LOG_INFO("[Parent] Writing message to shared memory...");
    memcpy(addr, SHM_MSG, strlen(SHM_MSG) + 1);

    // Wait for child to read and finish
    int status = 0;
    syscore_process_wait(pid, &status, 0);

    syscore_shm_unmap(addr, SHM_SIZE);
    syscore_shm_close(shm_fd);
    syscore_shm_destroy(SHM_NAME);
    SYSCORE_LOG_INFO("Shared Memory Demo completed successfully.");
  }

  return 0;
}
