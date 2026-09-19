#include "benchmark/benchmark.h"
#include "threading/threading.h"
#include <stdlib.h>

static void *dummy_thread_func(void *arg) {
  SYSCORE_UNUSED(arg);
  return NULL;
}

static syscore_error_t thread_step(void *user_data) {
  SYSCORE_UNUSED(user_data);
  syscore_thread_t thread;
  syscore_error_t err = syscore_thread_create(&thread, NULL, dummy_thread_func, NULL);
  if (err != SYSCORE_SUCCESS) return err;
  return syscore_thread_join(thread, NULL);
}

int main(void) {
  syscore_benchmark_config_t config;
  config.name = "Thread Spawn/Join Latency";
  config.warmup_iterations = 100;
  config.measured_iterations = 1000;
  config.setup = NULL;
  config.step = thread_step;
  config.teardown = NULL;
  config.user_data = NULL;

  return (syscore_benchmark_run(&config, NULL) == SYSCORE_SUCCESS) ? 0 : 1;
}
