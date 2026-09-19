#include "benchmark/benchmark.h"
#include "memory/shared_memory.h"
#include <stdlib.h>
#include <string.h>

#define BENCH_SHM_NAME "/syscore_bench_shm"
#define BENCH_SHM_SIZE 4096

typedef struct {
  syscore_shm_handle_t handle;
  void *addr;
} shm_bench_ctx_t;

static syscore_error_t shm_setup(void **user_data) {
  shm_bench_ctx_t *ctx = (shm_bench_ctx_t *)malloc(sizeof(shm_bench_ctx_t));
  if (!ctx) return SYSCORE_ERROR_OUT_OF_MEMORY;

  syscore_error_t err = syscore_shm_create(BENCH_SHM_NAME, BENCH_SHM_SIZE, 0666, &ctx->handle);
  if (err != SYSCORE_SUCCESS) {
    syscore_shm_destroy(BENCH_SHM_NAME);
    err = syscore_shm_create(BENCH_SHM_NAME, BENCH_SHM_SIZE, 0666, &ctx->handle);
    if (err != SYSCORE_SUCCESS) {
      free(ctx);
      return err;
    }
  }

  err = syscore_shm_map(ctx->handle, BENCH_SHM_SIZE, 1, &ctx->addr);
  if (err != SYSCORE_SUCCESS) {
    syscore_shm_close(ctx->handle);
    syscore_shm_destroy(BENCH_SHM_NAME);
    free(ctx);
    return err;
  }

  *user_data = ctx;
  return SYSCORE_SUCCESS;
}

static syscore_error_t shm_step(void *user_data) {
  shm_bench_ctx_t *ctx = (shm_bench_ctx_t *)user_data;
  volatile char *buf = (volatile char *)ctx->addr;

  /* Touch/access 64 bytes of shared memory */
  for (int i = 0; i < 64; i++) {
    buf[i] = (char)(i & 0xFF);
  }
  for (int i = 0; i < 64; i++) {
    SYSCORE_UNUSED(buf[i]);
  }

  return SYSCORE_SUCCESS;
}

static void shm_teardown(void *user_data) {
  shm_bench_ctx_t *ctx = (shm_bench_ctx_t *)user_data;
  if (ctx) {
    syscore_shm_unmap(ctx->addr, BENCH_SHM_SIZE);
    syscore_shm_close(ctx->handle);
    syscore_shm_destroy(BENCH_SHM_NAME);
    free(ctx);
  }
}

int main(void) {
  syscore_benchmark_config_t config;
  config.name = "Shared Memory Access Latency";
  config.warmup_iterations = 1000;
  config.measured_iterations = 100000;
  config.setup = shm_setup;
  config.step = shm_step;
  config.teardown = shm_teardown;
  config.user_data = NULL;

  return (syscore_benchmark_run(&config, NULL) == SYSCORE_SUCCESS) ? 0 : 1;
}
