#include "common/errors.h"
#include "common/logging.h"
#include "common/utils.h"
#include "memory/shared_memory.h"
#include "process/process.h"
#include "sync/semaphore.h"
#include <stdio.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define SHM_NAME "/syscore_shm_counter"
#define ITERATIONS 5

typedef struct {
  syscore_sem_t sem; // Use SysCore semaphore abstraction
  int counter;
} shared_state_t;

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting Shared Counter Demo (Shared Memory + Semaphore)");

  syscore_shm_destroy(SHM_NAME);

  syscore_shm_handle_t shm_fd;
  syscore_error_t err =
      syscore_shm_create(SHM_NAME, sizeof(shared_state_t), 0666, &shm_fd);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Failed to create shared memory: %s",
                      syscore_error_string(err));
    return 1;
  }

  shared_state_t *state = NULL;
  err = syscore_shm_map(shm_fd, sizeof(shared_state_t), 1, (void **)&state);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Map failed: %s", syscore_error_string(err));
    syscore_shm_close(shm_fd);
    syscore_shm_destroy(SHM_NAME);
    return 1;
  }

  // Initialize counter
  state->counter = 0;

  // Initialize the process-shared semaphore inside mapped memory.
  syscore_error_t sem_err = syscore_sem_init(&state->sem, 1, 1); // pshared = 1, initial value = 1
  if (sem_err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Process-shared sem_init failed: %s",
                      syscore_error_string(sem_err));
    syscore_shm_unmap(state, sizeof(shared_state_t));
    syscore_shm_close(shm_fd);
    syscore_shm_destroy(SHM_NAME);
    return 1;
  }

  syscore_pid_t pid;
  err = syscore_process_fork(&pid);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("Fork failed: %s", syscore_error_string(err));
    syscore_sem_destroy(&state->sem);
    syscore_shm_unmap(state, sizeof(shared_state_t));
    syscore_shm_close(shm_fd);
    syscore_shm_destroy(SHM_NAME);
    return 1;
  }

  if (pid == 0) {
    // Child process increments the counter
    for (int i = 0; i < ITERATIONS; i++) {
      syscore_sem_wait(&state->sem);
      state->counter++;
      SYSCORE_LOG_INFO("[Child] Incremented counter to: %d", state->counter);
      syscore_sem_post(&state->sem);
      struct timespec ts = {.tv_sec = 0, .tv_nsec = 50000000};
      nanosleep(&ts, NULL);
    }
    syscore_shm_unmap(state, sizeof(shared_state_t));
    return 0;
  } else {
    // Parent process also increments the counter
    for (int i = 0; i < ITERATIONS; i++) {
      syscore_sem_wait(&state->sem);
      state->counter++;
      SYSCORE_LOG_INFO("[Parent] Incremented counter to: %d", state->counter);
      syscore_sem_post(&state->sem);
      struct timespec ts = {.tv_sec = 0, .tv_nsec = 80000000};
      nanosleep(&ts, NULL);
    }

    // Wait for child
    int status = 0;
    syscore_process_wait(pid, &status, 0);

    SYSCORE_LOG_INFO("[Parent] Final Counter Value: %d (Expected: %d)",
                     state->counter, ITERATIONS * 2);

    // Cleanup
    syscore_sem_destroy(&state->sem);
    syscore_shm_unmap(state, sizeof(shared_state_t));
    syscore_shm_close(shm_fd);
    syscore_shm_destroy(SHM_NAME);

    SYSCORE_LOG_INFO("Shared Counter Demo completed successfully.");
  }

  return 0;
}
