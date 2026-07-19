#include "common/logging.h"
#include "sync/semaphore.h"
#include <stdio.h>
#include <time.h>

#define ITERATIONS 100000

int main(void) {
  syscore_log_init(SYSCORE_LOG_INFO);

  syscore_sem_t sem;
  syscore_sem_init(&sem, 0, 1);

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  for (int i = 0; i < ITERATIONS; i++) {
    syscore_sem_wait(&sem);
    syscore_sem_post(&sem);
  }

  clock_gettime(CLOCK_MONOTONIC, &end);

  double elapsed =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
  printf("Semaphore Wait/Post Loop Latency: %.3f us\n",
         (elapsed / ITERATIONS) * 1e6);

  syscore_sem_destroy(&sem);
  return 0;
}
