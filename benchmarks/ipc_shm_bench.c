#include "benchmark/benchmark.h"
#include "memory/shared_memory.h"
#include "sync/semaphore.h"
#include "sync/sync.h"
#include "threading/threading.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHM_SUITE_NAME "/syscore_ipc_shm_suite"
#define SHM_SUITE_SIZE 4096

typedef struct {
  syscore_shm_handle_t handle;
  void *addr;
  syscore_mutex_t mutex;
  syscore_sem_t sem_start;
  syscore_sem_t sem_done;
  syscore_thread_t threads[8];
  size_t threads_count;
  size_t threads_created;
  size_t ops_per_thread;
  volatile int stop_flag;
  volatile size_t shared_counter;
} shm_bench_ctx_t;

typedef struct {
  shm_bench_ctx_t *ctx;
} worker_arg_t;

static void *shm_worker_thread(void *arg) {
  worker_arg_t *warg = (worker_arg_t *)arg;
  shm_bench_ctx_t *ctx = warg->ctx;
  free(warg);

  while (1) {
    syscore_error_t err = syscore_sem_wait(&ctx->sem_start);
    if (err != SYSCORE_SUCCESS || ctx->stop_flag) {
      break;
    }

    for (size_t i = 0; i < ctx->ops_per_thread; i++) {
      syscore_mutex_lock(&ctx->mutex);
      ctx->shared_counter++;
      volatile char *buf = (volatile char *)ctx->addr;
      buf[0] = (char)(ctx->shared_counter & 0xFF);
      syscore_mutex_unlock(&ctx->mutex);
    }

    syscore_sem_post(&ctx->sem_done);
  }

  return NULL;
}

static syscore_error_t shm_suite_setup(void **user_data) {
  shm_bench_ctx_t *ctx = (shm_bench_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  syscore_shm_destroy(SHM_SUITE_NAME);

  syscore_error_t err =
      syscore_shm_create(SHM_SUITE_NAME, SHM_SUITE_SIZE, 0666, &ctx->handle);
  if (err != SYSCORE_SUCCESS) return err;

  err = syscore_shm_map(ctx->handle, SHM_SUITE_SIZE, 1, &ctx->addr);
  if (err != SYSCORE_SUCCESS) {
    syscore_shm_close(ctx->handle);
    syscore_shm_destroy(SHM_SUITE_NAME);
    return err;
  }

  err = syscore_mutex_init(&ctx->mutex);
  if (err != SYSCORE_SUCCESS) {
    syscore_shm_unmap(ctx->addr, SHM_SUITE_SIZE);
    syscore_shm_close(ctx->handle);
    syscore_shm_destroy(SHM_SUITE_NAME);
    return err;
  }

  ctx->shared_counter = 0;
  ctx->threads_created = 0;
  ctx->stop_flag = 0;

  if (ctx->threads_count > 1) {
    err = syscore_sem_init(&ctx->sem_start, 0, 0);
    if (err != SYSCORE_SUCCESS) {
      syscore_mutex_destroy(&ctx->mutex);
      syscore_shm_unmap(ctx->addr, SHM_SUITE_SIZE);
      syscore_shm_close(ctx->handle);
      syscore_shm_destroy(SHM_SUITE_NAME);
      return err;
    }

    err = syscore_sem_init(&ctx->sem_done, 0, 0);
    if (err != SYSCORE_SUCCESS) {
      syscore_sem_destroy(&ctx->sem_start);
      syscore_mutex_destroy(&ctx->mutex);
      syscore_shm_unmap(ctx->addr, SHM_SUITE_SIZE);
      syscore_shm_close(ctx->handle);
      syscore_shm_destroy(SHM_SUITE_NAME);
      return err;
    }

    ctx->ops_per_thread = 1;
    for (size_t i = 0; i < ctx->threads_count; i++) {
      worker_arg_t *arg = (worker_arg_t *)malloc(sizeof(worker_arg_t));
      if (!arg) {
        err = SYSCORE_ERROR_OUT_OF_MEMORY;
        break;
      }
      arg->ctx = ctx;

      err = syscore_thread_create(&ctx->threads[i], NULL, shm_worker_thread, arg);
      if (err != SYSCORE_SUCCESS) {
        free(arg);
        break;
      }
      ctx->threads_created++;
    }

    if (err != SYSCORE_SUCCESS) {
      ctx->stop_flag = 1;
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_sem_post(&ctx->sem_start);
      }
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_thread_join(ctx->threads[i], NULL);
      }
      syscore_sem_destroy(&ctx->sem_start);
      syscore_sem_destroy(&ctx->sem_done);
      syscore_mutex_destroy(&ctx->mutex);
      syscore_shm_unmap(ctx->addr, SHM_SUITE_SIZE);
      syscore_shm_close(ctx->handle);
      syscore_shm_destroy(SHM_SUITE_NAME);
      return err;
    }
  }

  return SYSCORE_SUCCESS;
}

static syscore_error_t shm_suite_step(void *user_data) {
  shm_bench_ctx_t *ctx = (shm_bench_ctx_t *)user_data;

  if (ctx->threads_count <= 1) {
    /* Single-threaded access */
    syscore_mutex_lock(&ctx->mutex);
    ctx->shared_counter++;
    volatile char *buf = (volatile char *)ctx->addr;
    buf[0] = (char)(ctx->shared_counter & 0xFF);
    syscore_mutex_unlock(&ctx->mutex);
  } else {
    /* Multi-threaded access under contention */
    for (size_t i = 0; i < ctx->threads_count; i++) {
      syscore_sem_post(&ctx->sem_start);
    }

    for (size_t i = 0; i < ctx->threads_count; i++) {
      syscore_sem_wait(&ctx->sem_done);
    }
  }

  return SYSCORE_SUCCESS;
}

static void shm_suite_teardown(void *user_data) {
  shm_bench_ctx_t *ctx = (shm_bench_ctx_t *)user_data;
  if (ctx) {
    if (ctx->threads_created > 0) {
      ctx->stop_flag = 1;
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_sem_post(&ctx->sem_start);
      }
      for (size_t i = 0; i < ctx->threads_created; i++) {
        syscore_thread_join(ctx->threads[i], NULL);
      }
      syscore_sem_destroy(&ctx->sem_start);
      syscore_sem_destroy(&ctx->sem_done);
    }
    syscore_mutex_destroy(&ctx->mutex);
    syscore_shm_unmap(ctx->addr, SHM_SUITE_SIZE);
    syscore_shm_close(ctx->handle);
    syscore_shm_destroy(SHM_SUITE_NAME);
  }
}

int main(void) {
  static const size_t thread_counts[] = {1, 2, 4, 8};
  static const char *names[] = {
      "Shared Memory Access (1 Thread)",
      "Shared Memory Access (2 Threads Contended)",
      "Shared Memory Access (4 Threads Contended)",
      "Shared Memory Access (8 Threads Contended)"};

  syscore_error_t status = SYSCORE_SUCCESS;

  for (size_t i = 0; i < SYSCORE_ARRAY_SIZE(thread_counts); i++) {
    shm_bench_ctx_t ctx;
    ctx.threads_count = thread_counts[i];

    syscore_benchmark_config_t config;
    config.name = names[i];
    config.warmup_iterations = (ctx.threads_count > 2) ? 50 : 200;
    config.measured_iterations = (ctx.threads_count > 2) ? 500 : 5000;
    config.setup = shm_suite_setup;
    config.step = shm_suite_step;
    config.teardown = shm_suite_teardown;
    config.user_data = &ctx;

    syscore_error_t err = syscore_benchmark_run(&config, NULL);
    if (err != SYSCORE_SUCCESS) {
      status = err;
    }
  }

  return (status == SYSCORE_SUCCESS) ? 0 : 1;
}
