#include "benchmark/benchmark.h"
#include "sync/sync.h"
#include "threading/threading.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  syscore_mutex_t mutex;
  syscore_cond_t cond;
  volatile int ready;
} condvar_bench_ctx_t;

static void *condvar_producer_thread(void *arg) {
  condvar_bench_ctx_t *ctx = (condvar_bench_ctx_t *)arg;

  syscore_mutex_lock(&ctx->mutex);
  ctx->ready = 1;
  syscore_cond_signal(&ctx->cond);
  syscore_mutex_unlock(&ctx->mutex);

  return NULL;
}

static syscore_error_t condvar_bench_setup(void **user_data) {
  condvar_bench_ctx_t *ctx = (condvar_bench_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  syscore_error_t err = syscore_mutex_init(&ctx->mutex);
  if (err != SYSCORE_SUCCESS) return err;

  err = syscore_cond_init(&ctx->cond);
  if (err != SYSCORE_SUCCESS) {
    syscore_mutex_destroy(&ctx->mutex);
    return err;
  }

  ctx->ready = 0;
  return SYSCORE_SUCCESS;
}

static syscore_error_t condvar_bench_step(void *user_data) {
  condvar_bench_ctx_t *ctx = (condvar_bench_ctx_t *)user_data;

  ctx->ready = 0;
  syscore_thread_t prod;
  syscore_error_t err = syscore_thread_create(&prod, NULL, condvar_producer_thread, ctx);
  if (err != SYSCORE_SUCCESS) return err;

  syscore_mutex_lock(&ctx->mutex);
  while (!ctx->ready) {
    syscore_cond_wait(&ctx->cond, &ctx->mutex);
  }
  ctx->ready = 0;
  syscore_mutex_unlock(&ctx->mutex);

  syscore_thread_join(prod, NULL);
  return SYSCORE_SUCCESS;
}

static void condvar_bench_teardown(void *user_data) {
  condvar_bench_ctx_t *ctx = (condvar_bench_ctx_t *)user_data;
  if (ctx) {
    syscore_cond_destroy(&ctx->cond);
    syscore_mutex_destroy(&ctx->mutex);
  }
}

int main(void) {
  condvar_bench_ctx_t ctx;

  syscore_benchmark_config_t config;
  config.name = "Condition Variable Wait/Signal Latency (Producer-Consumer)";
  config.warmup_iterations = 200;
  config.measured_iterations = 2000;
  config.setup = condvar_bench_setup;
  config.step = condvar_bench_step;
  config.teardown = condvar_bench_teardown;
  config.user_data = &ctx;

  syscore_error_t err = syscore_benchmark_run(&config, NULL);
  return (err == SYSCORE_SUCCESS) ? 0 : 1;
}
