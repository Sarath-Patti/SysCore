#include "benchmark/benchmark.h"
#include "ipc/ipc.h"
#include <stdlib.h>

typedef struct {
  syscore_ipc_handle_t rd;
  syscore_ipc_handle_t wr;
} pipe_bench_ctx_t;

static syscore_error_t pipe_setup(void **user_data) {
  pipe_bench_ctx_t *ctx = (pipe_bench_ctx_t *)malloc(sizeof(pipe_bench_ctx_t));
  if (!ctx) return SYSCORE_ERROR_OUT_OF_MEMORY;

  syscore_error_t err = syscore_ipc_pipe_create(&ctx->rd, &ctx->wr);
  if (err != SYSCORE_SUCCESS) {
    free(ctx);
    return err;
  }

  *user_data = ctx;
  return SYSCORE_SUCCESS;
}

static syscore_error_t pipe_step(void *user_data) {
  pipe_bench_ctx_t *ctx = (pipe_bench_ctx_t *)user_data;
  char val = 'x';
  size_t written = 0;
  size_t read_bytes = 0;

  syscore_error_t err = syscore_ipc_write(ctx->wr, &val, 1, &written);
  if (err != SYSCORE_SUCCESS) return err;

  return syscore_ipc_read(ctx->rd, &val, 1, &read_bytes);
}

static void pipe_teardown(void *user_data) {
  pipe_bench_ctx_t *ctx = (pipe_bench_ctx_t *)user_data;
  if (ctx) {
    syscore_ipc_close(ctx->rd);
    syscore_ipc_close(ctx->wr);
    free(ctx);
  }
}

int main(void) {
  syscore_benchmark_config_t config;
  config.name = "Pipe Round-Trip Communication Latency";
  config.warmup_iterations = 1000;
  config.measured_iterations = 10000;
  config.setup = pipe_setup;
  config.step = pipe_step;
  config.teardown = pipe_teardown;
  config.user_data = NULL;

  return (syscore_benchmark_run(&config, NULL) == SYSCORE_SUCCESS) ? 0 : 1;
}
