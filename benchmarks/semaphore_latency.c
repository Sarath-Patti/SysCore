#include "benchmark/benchmark.h"
#include "sync/semaphore.h"
#include <stdlib.h>

typedef struct {
  syscore_sem_t sem;
} sem_bench_ctx_t;

static syscore_error_t sem_setup(void **user_data) {
  sem_bench_ctx_t *ctx = (sem_bench_ctx_t *)malloc(sizeof(sem_bench_ctx_t));
  if (!ctx) return SYSCORE_ERROR_OUT_OF_MEMORY;

  syscore_error_t err = syscore_sem_init(&ctx->sem, 0, 1);
  if (err != SYSCORE_SUCCESS) {
    free(ctx);
    return err;
  }

  *user_data = ctx;
  return SYSCORE_SUCCESS;
}

static syscore_error_t sem_step(void *user_data) {
  sem_bench_ctx_t *ctx = (sem_bench_ctx_t *)user_data;
  syscore_error_t err = syscore_sem_wait(&ctx->sem);
  if (err != SYSCORE_SUCCESS) return err;
  return syscore_sem_post(&ctx->sem);
}

static void sem_teardown(void *user_data) {
  sem_bench_ctx_t *ctx = (sem_bench_ctx_t *)user_data;
  if (ctx) {
    syscore_sem_destroy(&ctx->sem);
    free(ctx);
  }
}

int main(void) {
  syscore_benchmark_config_t config;
  config.name = "Semaphore Wait/Post Latency";
  config.warmup_iterations = 1000;
  config.measured_iterations = 100000;
  config.setup = sem_setup;
  config.step = sem_step;
  config.teardown = sem_teardown;
  config.user_data = NULL;

  return (syscore_benchmark_run(&config, NULL) == SYSCORE_SUCCESS) ? 0 : 1;
}
