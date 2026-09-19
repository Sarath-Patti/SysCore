#include "benchmark/benchmark.h"
#include "sync/semaphore.h"
#include "threading/threading.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  syscore_sem_t sem;
  syscore_sem_t worker_start[8];
  syscore_sem_t sem_done;
  syscore_thread_t threads[8];
  size_t threads_count;
  size_t threads_created;
  size_t ops_per_thread;
  volatile int stop_flag;
  volatile size_t counter;
} sem_contention_ctx_t;

typedef struct {
  sem_contention_ctx_t *ctx;
  size_t thread_index;
} sem_worker_arg_t;

static void *sem_worker_thread(void *arg) {
  sem_worker_arg_t *warg = (sem_worker_arg_t *)arg;
  sem_contention_ctx_t *ctx = warg->ctx;
  size_t idx = warg->thread_index;
  free(warg);

  while (1) {
    syscore_error_t err = syscore_sem_wait(&ctx->worker_start[idx]);
    if (err != SYSCORE_SUCCESS || ctx->stop_flag) {
      break;
    }

    for (size_t i = 0; i < ctx->ops_per_thread; i++) {
      syscore_sem_wait(&ctx->sem);
      ctx->counter++;
      syscore_sem_post(&ctx->sem);
    }

    syscore_sem_post(&ctx->sem_done);
  }

  return NULL;
}

static syscore_error_t sem_bench_setup(void **user_data) {
  sem_contention_ctx_t *ctx = (sem_contention_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  syscore_error_t err = syscore_sem_init(&ctx->sem, 0, 1);
  if (err != SYSCORE_SUCCESS) return err;

  ctx->counter = 0;
  ctx->threads_created = 0;
  ctx->stop_flag = 0;

  if (ctx->threads_count > 1) {
    for (size_t i = 0; i < ctx->threads_count; i++) {
      err = syscore_sem_init(&ctx->worker_start[i], 0, 0);
      if (err != SYSCORE_SUCCESS) {
        for (size_t j = 0; j < i; j++) {
          syscore_sem_destroy(&ctx->worker_start[j]);
        }
        syscore_sem_destroy(&ctx->sem);
        return err;
      }
    }

    err = syscore_sem_init(&ctx->sem_done, 0, 0);
    if (err != SYSCORE_SUCCESS) {
      for (size_t i = 0; i < ctx->threads_count; i++) {
        syscore_sem_destroy(&ctx->worker_start[i]);
      }
      syscore_sem_destroy(&ctx->sem);
      return err;
    }

    ctx->ops_per_thread = 5;
    for (size_t i = 0; i < ctx->threads_count; i++) {
      sem_worker_arg_t *arg = (sem_worker_arg_t *)malloc(sizeof(sem_worker_arg_t));
      if (!arg) {
        err = SYSCORE_ERROR_OUT_OF_MEMORY;
        break;
      }
      arg->ctx = ctx;
      arg->thread_index = i;

      err = syscore_thread_create(&ctx->threads[i], NULL, sem_worker_thread, arg);
      if (err != SYSCORE_SUCCESS) {
        free(arg);
        break;
      }
      ctx->threads_created++;
    }

    if (err != SYSCORE_SUCCESS) {
      ctx->stop_flag = 1;
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_sem_post(&ctx->worker_start[i]);
      }
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_thread_join(ctx->threads[i], NULL);
        syscore_sem_destroy(&ctx->worker_start[i]);
      }
      syscore_sem_destroy(&ctx->sem_done);
      syscore_sem_destroy(&ctx->sem);
      return err;
    }
  }

  return SYSCORE_SUCCESS;
}

static syscore_error_t sem_bench_step(void *user_data) {
  sem_contention_ctx_t *ctx = (sem_contention_ctx_t *)user_data;

  if (ctx->threads_count <= 1) {
    syscore_sem_wait(&ctx->sem);
    ctx->counter++;
    syscore_sem_post(&ctx->sem);
  } else {
    for (size_t i = 0; i < ctx->threads_count; i++) {
      syscore_sem_post(&ctx->worker_start[i]);
    }

    for (size_t i = 0; i < ctx->threads_count; i++) {
      syscore_sem_wait(&ctx->sem_done);
    }
  }

  return SYSCORE_SUCCESS;
}

static void sem_bench_teardown(void *user_data) {
  sem_contention_ctx_t *ctx = (sem_contention_ctx_t *)user_data;
  if (ctx) {
    if (ctx->threads_created > 0) {
      ctx->stop_flag = 1;
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_sem_post(&ctx->worker_start[i]);
      }
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_thread_join(ctx->threads[i], NULL);
        syscore_sem_destroy(&ctx->worker_start[i]);
      }
      syscore_sem_destroy(&ctx->sem_done);
    }
    syscore_sem_destroy(&ctx->sem);
  }
}

int main(void) {
  static const size_t thread_counts[] = {1, 2, 4, 8};
  static const char *names[] = {
      "Semaphore Wait/Post (1 Thread Uncontended)",
      "Semaphore Wait/Post (2 Threads Contended)",
      "Semaphore Wait/Post (4 Threads Contended)",
      "Semaphore Wait/Post (8 Threads Contended)"};

  syscore_error_t status = SYSCORE_SUCCESS;

  for (size_t i = 0; i < SYSCORE_ARRAY_SIZE(thread_counts); i++) {
    sem_contention_ctx_t ctx;
    ctx.threads_count = thread_counts[i];

    syscore_benchmark_config_t config;
    config.name = names[i];
    config.warmup_iterations = (ctx.threads_count > 2) ? 200 : 500;
    config.measured_iterations = (ctx.threads_count > 2) ? 2000 : 5000;
    config.setup = sem_bench_setup;
    config.step = sem_bench_step;
    config.teardown = sem_bench_teardown;
    config.user_data = &ctx;

    syscore_error_t err = syscore_benchmark_run(&config, NULL);
    if (err != SYSCORE_SUCCESS) {
      status = err;
    }
  }

  return (status == SYSCORE_SUCCESS) ? 0 : 1;
}
