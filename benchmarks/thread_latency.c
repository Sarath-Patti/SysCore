#include "common/logging.h"
#include "threading/threading.h"
#include <stdio.h>
#include <time.h>

#define ITERATIONS 1000

static void *dummy_thread_func(void *arg) {
  (void)arg;
  return NULL;
}

int main(void) {
  syscore_log_init(SYSCORE_LOG_INFO);

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  for (int i = 0; i < ITERATIONS; i++) {
    syscore_thread_t thread;
    syscore_thread_create(&thread, NULL, dummy_thread_func, NULL);
    syscore_thread_join(thread, NULL);
  }

  clock_gettime(CLOCK_MONOTONIC, &end);

  double elapsed =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
  printf("Thread Spawn/Join Latency: %.3f us\n", (elapsed / ITERATIONS) * 1e6);

  return 0;
}
