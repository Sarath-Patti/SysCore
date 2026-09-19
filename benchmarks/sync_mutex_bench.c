#include "benchmark/benchmark.h"
#include "sync/sync.h"
#include "threading/threading.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  syscore_mutex_t mutex;
  size_t threads_count;
  size_t ops_per_thread;
  volatile size_t counter;
} mutex_contention_ctx_t;

typedef struct {
  mutex_contention_ctx_t *ctx;
} mutex_worker_arg_t;

static void *mutex_worker_thread(void *arg) {
  mutex_worker_arg_t *warg = (mutex_worker_arg_t *)arg;
  mutex_contention_ctx_t *ctx = warg->ctx;

  for (size_t i = 0; i < ctx->ops_per_thread; i++) {
    syscore_mutex_lock(&ctx->mutex);
    ctx->counter++;
    syscore_mutex_unlock(&ctx->mutex);
  }

  return NULL;
}

static syscore_error_t mutex_bench_setup(void **user_data) {
  mutex_contention_ctx_t *ctx = (mutex_contention_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  syscore_error_t err = syscore_mutex_init(&ctx->mutex);
  if (err != SYSCORE_SUCCESS) return err;

  ctx->counter = 0;
  return SYSCORE_SUCCESS;
}

static syscore_error_t mutex_bench_step(void *user_data) {
  mutex_contention_ctx_t *ctx = (mutex_contention_ctx_t *)user_data;

  if (ctx->threads_count <= 1) {
    syscore_mutex_lock(&ctx->mutex);
    ctx->counter++;
    syscore_mutex_unlock(&ctx->mutex);
  } else {
    syscore_thread_t threads[8];
    mutex_worker_arg_t args[8];
    ctx->ops_per_thread = 10;

    for (size_t i = 0; i < ctx->threads_count; i++) {
      args[i].ctx = ctx;
      syscore_error_t err =
          syscore_thread_create(&threads[i], NULL, mutex_worker_thread, &args[i]);
      if (err != SYSCORE_SUCCESS) return err;
    }

    for (size_t i = 0; i < ctx->threads_count; i++) {
      syscore_thread_join(threads[i], NULL);
    }
  }

  return SYSCORE_SUCCESS;
}

static void mutex_bench_teardown(void *user_data) {
  mutex_contention_ctx_t *ctx = (mutex_contention_ctx_t *)user_data;
  if (ctx) {
    syscore_mutex_destroy(&ctx->mutex);
  }
}

int main(void) {
  static const size_t thread_counts[] = {1, 2, 4, 8};
  static const char *names[] = {
      "Mutex Lock/Unlock (1 Thread Uncontended)",
      "Mutex Lock/Unlock (2 Threads Contended)",
      "Mutex Lock/Unlock (4 Threads Contended)",
      "Mutex Lock/Unlock (8 Threads Contended)"};

  syscore_error_t status = SYSCORE_SUCCESS;

  for (size_t i = 0; i < SYSCORE_ARRAY_SIZE(thread_counts); i++) {
    mutex_contention_ctx_t ctx;
    ctx.threads_count = thread_counts[i];

    syscore_benchmark_config_t config;
    config.name = names[i];
    config.warmup_iterations = (ctx.threads_count > 2) ? 50 : 500;
    config.measured_iterations = (ctx.threads_count > 2) ? 500 : 10000;
    config.setup = mutex_bench_setup;
    config.step = mutex_bench_step;
    config.teardown = mutex_bench_teardown;
    config.user_data = &ctx;

    syscore_error_t err = syscore_benchmark_run(&config, NULL);
    if (err != SYSCORE_SUCCESS) {
      status = err;
    }
  }

  return (status == SYSCORE_SUCCESS) ? 0 : 1;
}
