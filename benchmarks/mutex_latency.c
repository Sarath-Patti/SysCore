#include "benchmark/benchmark.h"
#include "sync/sync.h"
#include <stdlib.h>

typedef struct {
  syscore_mutex_t mutex;
} mutex_bench_ctx_t;

static syscore_error_t mutex_setup(void **user_data) {
  mutex_bench_ctx_t *ctx = (mutex_bench_ctx_t *)malloc(sizeof(mutex_bench_ctx_t));
  if (!ctx) return SYSCORE_ERROR_OUT_OF_MEMORY;

  syscore_error_t err = syscore_mutex_init(&ctx->mutex);
  if (err != SYSCORE_SUCCESS) {
    free(ctx);
    return err;
  }

  *user_data = ctx;
  return SYSCORE_SUCCESS;
}

static syscore_error_t mutex_step(void *user_data) {
  mutex_bench_ctx_t *ctx = (mutex_bench_ctx_t *)user_data;
  syscore_error_t err = syscore_mutex_lock(&ctx->mutex);
  if (err != SYSCORE_SUCCESS) return err;
  return syscore_mutex_unlock(&ctx->mutex);
}

static void mutex_teardown(void *user_data) {
  mutex_bench_ctx_t *ctx = (mutex_bench_ctx_t *)user_data;
  if (ctx) {
    syscore_mutex_destroy(&ctx->mutex);
    free(ctx);
  }
}

int main(void) {
  syscore_benchmark_config_t config;
  config.name = "Mutex Lock/Unlock Latency";
  config.warmup_iterations = 1000;
  config.measured_iterations = 100000;
  config.setup = mutex_setup;
  config.step = mutex_step;
  config.teardown = mutex_teardown;
  config.user_data = NULL;

  return (syscore_benchmark_run(&config, NULL) == SYSCORE_SUCCESS) ? 0 : 1;
}
