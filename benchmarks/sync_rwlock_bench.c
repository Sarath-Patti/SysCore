#include "benchmark/benchmark.h"
#include "sync/sync.h"
#include "threading/threading.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  syscore_rwlock_t rwlock;
  size_t reader_threads;
  size_t writer_threads;
  size_t ops_per_thread;
  volatile size_t shared_counter;
} rwlock_bench_ctx_t;

typedef struct {
  rwlock_bench_ctx_t *ctx;
  int is_writer;
} rwlock_worker_arg_t;

static void *rwlock_worker_thread(void *arg) {
  rwlock_worker_arg_t *warg = (rwlock_worker_arg_t *)arg;
  rwlock_bench_ctx_t *ctx = warg->ctx;

  if (warg->is_writer) {
    for (size_t i = 0; i < ctx->ops_per_thread; i++) {
      syscore_rwlock_wrlock(&ctx->rwlock);
      ctx->shared_counter++;
      syscore_rwlock_unlock(&ctx->rwlock);
    }
  } else {
    for (size_t i = 0; i < ctx->ops_per_thread; i++) {
      syscore_rwlock_rdlock(&ctx->rwlock);
      (void)ctx->shared_counter;
      syscore_rwlock_unlock(&ctx->rwlock);
    }
  }

  return NULL;
}

static syscore_error_t rwlock_bench_setup(void **user_data) {
  rwlock_bench_ctx_t *ctx = (rwlock_bench_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  syscore_error_t err = syscore_rwlock_init(&ctx->rwlock);
  if (err != SYSCORE_SUCCESS) return err;

  ctx->shared_counter = 0;
  return SYSCORE_SUCCESS;
}

static syscore_error_t rwlock_bench_step(void *user_data) {
  rwlock_bench_ctx_t *ctx = (rwlock_bench_ctx_t *)user_data;
  size_t total_threads = ctx->reader_threads + ctx->writer_threads;

  if (total_threads <= 1) {
    if (ctx->writer_threads > 0) {
      syscore_rwlock_wrlock(&ctx->rwlock);
      ctx->shared_counter++;
      syscore_rwlock_unlock(&ctx->rwlock);
    } else {
      syscore_rwlock_rdlock(&ctx->rwlock);
      (void)ctx->shared_counter;
      syscore_rwlock_unlock(&ctx->rwlock);
    }
  } else {
    syscore_thread_t threads[16];
    rwlock_worker_arg_t args[16];
    ctx->ops_per_thread = 10;

    size_t idx = 0;
    for (size_t i = 0; i < ctx->reader_threads; i++, idx++) {
      args[idx].ctx = ctx;
      args[idx].is_writer = 0;
      syscore_error_t err =
          syscore_thread_create(&threads[idx], NULL, rwlock_worker_thread, &args[idx]);
      if (err != SYSCORE_SUCCESS) return err;
    }

    for (size_t i = 0; i < ctx->writer_threads; i++, idx++) {
      args[idx].ctx = ctx;
      args[idx].is_writer = 1;
      syscore_error_t err =
          syscore_thread_create(&threads[idx], NULL, rwlock_worker_thread, &args[idx]);
      if (err != SYSCORE_SUCCESS) return err;
    }

    for (size_t i = 0; i < total_threads; i++) {
      syscore_thread_join(threads[i], NULL);
    }
  }

  return SYSCORE_SUCCESS;
}

static void rwlock_bench_teardown(void *user_data) {
  rwlock_bench_ctx_t *ctx = (rwlock_bench_ctx_t *)user_data;
  if (ctx) {
    syscore_rwlock_destroy(&ctx->rwlock);
  }
}

int main(void) {
  struct {
    const char *name;
    size_t reader_threads;
    size_t writer_threads;
  } configs[] = {
      {"Read-Write Lock (Single Reader Uncontended)", 1, 0},
      {"Read-Write Lock Read-Heavy (8 Readers, 1 Writer)", 8, 1},
      {"Read-Write Lock Write-Heavy (1 Reader, 4 Writers)", 1, 4},
      {"Read-Write Lock Mixed (4 Readers, 4 Writers)", 4, 4}};

  syscore_error_t status = SYSCORE_SUCCESS;

  for (size_t i = 0; i < SYSCORE_ARRAY_SIZE(configs); i++) {
    rwlock_bench_ctx_t ctx;
    ctx.reader_threads = configs[i].reader_threads;
    ctx.writer_threads = configs[i].writer_threads;

    syscore_benchmark_config_t bench_config;
    bench_config.name = configs[i].name;
    bench_config.warmup_iterations = (ctx.reader_threads + ctx.writer_threads > 2) ? 50 : 500;
    bench_config.measured_iterations = (ctx.reader_threads + ctx.writer_threads > 2) ? 500 : 5000;
    bench_config.setup = rwlock_bench_setup;
    bench_config.step = rwlock_bench_step;
    bench_config.teardown = rwlock_bench_teardown;
    bench_config.user_data = &ctx;

    syscore_error_t err = syscore_benchmark_run(&bench_config, NULL);
    if (err != SYSCORE_SUCCESS) {
      status = err;
    }
  }

  return (status == SYSCORE_SUCCESS) ? 0 : 1;
}
