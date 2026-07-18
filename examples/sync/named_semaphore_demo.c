#include "common/errors.h"
#include "common/logging.h"
#include "common/utils.h"
#include "process/process.h"
#include "sync/semaphore.h"
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define SEM_NAME "/syscore_sem_demo"

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting Named Semaphore Demo");

  // Clean up any stale named semaphore
  syscore_sem_unlink(SEM_NAME);

  syscore_sem_t sem;
  syscore_error_t err =
      syscore_sem_open(SEM_NAME, 0666, 0, &sem); // Initial value 0 (blocked)
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Failed to open named semaphore: %s",
                      syscore_error_string(err));
    return 1;
  }

  syscore_pid_t pid;
  err = syscore_process_fork(&pid);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Fork failed: %s", syscore_error_string(err));
    syscore_sem_close(&sem);
    syscore_sem_unlink(SEM_NAME);
    return 1;
  }

  if (pid == 0) {
    // Child process waits on the parent
    SYSCORE_LOG_INFO("[Child] Waiting on named semaphore...");
    syscore_sem_wait(&sem);

    SYSCORE_LOG_INFO("[Child] Received signal from parent. Child proceeding...");

    syscore_sem_close(&sem);
    return 0;
  } else {
    // Parent process sleeps, then posts
    SYSCORE_LOG_INFO("[Parent] Doing work...");
    usleep(500000); // 500ms

    SYSCORE_LOG_INFO("[Parent] Signaling child via named semaphore...");
    syscore_sem_post(&sem);

    // Wait for child to complete
    int status = 0;
    syscore_process_wait(pid, &status, 0);

    syscore_sem_close(&sem);
    syscore_sem_unlink(SEM_NAME);
    SYSCORE_LOG_INFO("Named Semaphore Demo completed successfully.");
  }

  return 0;
}
