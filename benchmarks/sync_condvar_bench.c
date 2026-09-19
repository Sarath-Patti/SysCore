#include "benchmark/benchmark.h"
#include "sync/semaphore.h"
#include "sync/sync.h"
#include "threading/threading.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  syscore_mutex_t mutex;
  syscore_cond_t cond;
  syscore_sem_t sem_start;
  syscore_sem_t sem_done;
  syscore_thread_t producer_thread;
  int producer_created;
  volatile int stop_flag;
  volatile int ready;
} condvar_bench_ctx_t;

static void *condvar_producer_thread(void *arg) {
  condvar_bench_ctx_t *ctx = (condvar_bench_ctx_t *)arg;

  while (1) {
    syscore_error_t err = syscore_sem_wait(&ctx->sem_start);
    if (err != SYSCORE_SUCCESS || ctx->stop_flag) {
      break;
    }

    syscore_mutex_lock(&ctx->mutex);
    ctx->ready = 1;
    syscore_cond_signal(&ctx->cond);
    syscore_mutex_unlock(&ctx->mutex);

    syscore_sem_post(&ctx->sem_done);
  }

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

  err = syscore_sem_init(&ctx->sem_start, 0, 0);
  if (err != SYSCORE_SUCCESS) {
    syscore_cond_destroy(&ctx->cond);
    syscore_mutex_destroy(&ctx->mutex);
    return err;
  }

  err = syscore_sem_init(&ctx->sem_done, 0, 0);
  if (err != SYSCORE_SUCCESS) {
    syscore_sem_destroy(&ctx->sem_start);
    syscore_cond_destroy(&ctx->cond);
    syscore_mutex_destroy(&ctx->mutex);
    return err;
  }

  ctx->ready = 0;
  ctx->stop_flag = 0;
  ctx->producer_created = 0;

  err = syscore_thread_create(&ctx->producer_thread, NULL, condvar_producer_thread, ctx);
  if (err != SYSCORE_SUCCESS) {
    syscore_sem_destroy(&ctx->sem_start);
    syscore_sem_destroy(&ctx->sem_done);
    syscore_cond_destroy(&ctx->cond);
    syscore_mutex_destroy(&ctx->mutex);
    return err;
  }
  ctx->producer_created = 1;

  return SYSCORE_SUCCESS;
}

static syscore_error_t condvar_bench_step(void *user_data) {
  condvar_bench_ctx_t *ctx = (condvar_bench_ctx_t *)user_data;

  syscore_mutex_lock(&ctx->mutex);
  ctx->ready = 0;
  syscore_sem_post(&ctx->sem_start);

  while (!ctx->ready) {
    syscore_cond_wait(&ctx->cond, &ctx->mutex);
  }
  ctx->ready = 0;
  syscore_mutex_unlock(&ctx->mutex);

  syscore_sem_wait(&ctx->sem_done);
  return SYSCORE_SUCCESS;
}

static void condvar_bench_teardown(void *user_data) {
  condvar_bench_ctx_t *ctx = (condvar_bench_ctx_t *)user_data;
  if (ctx) {
    if (ctx->producer_created) {
      ctx->stop_flag = 1;
      syscore_sem_post(&ctx->sem_start);
      syscore_thread_join(ctx->producer_thread, NULL);
      syscore_sem_destroy(&ctx->sem_start);
      syscore_sem_destroy(&ctx->sem_done);
    }
    syscore_cond_destroy(&ctx->cond);
    syscore_mutex_destroy(&ctx->mutex);
  }
}

int main(void) {
  condvar_bench_ctx_t ctx;

  syscore_benchmark_config_t config;
  config.name = "Condition Variable Wait/Signal Latency (Producer-Consumer)";
  config.warmup_iterations = 500;
  config.measured_iterations = 5000;
  config.setup = condvar_bench_setup;
  config.step = condvar_bench_step;
  config.teardown = condvar_bench_teardown;
  config.user_data = &ctx;

  syscore_error_t err = syscore_benchmark_run(&config, NULL);
  return (err == SYSCORE_SUCCESS) ? 0 : 1;
}
