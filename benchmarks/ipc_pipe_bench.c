#include "benchmark/benchmark.h"
#include "ipc/ipc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  syscore_ipc_handle_t rd;
  syscore_ipc_handle_t wr;
  size_t payload_size;
  char *write_buf;
  char *read_buf;
} pipe_payload_ctx_t;

static syscore_error_t pipe_payload_setup(void **user_data) {
  pipe_payload_ctx_t *ctx = (pipe_payload_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  ctx->write_buf = (char *)malloc(ctx->payload_size);
  ctx->read_buf = (char *)malloc(ctx->payload_size);
  if (!ctx->write_buf || !ctx->read_buf) {
    if (ctx->write_buf) free(ctx->write_buf);
    if (ctx->read_buf) free(ctx->read_buf);
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  }

  memset(ctx->write_buf, 'A', ctx->payload_size);

  syscore_error_t err = syscore_ipc_pipe_create(&ctx->rd, &ctx->wr);
  if (err != SYSCORE_SUCCESS) {
    free(ctx->write_buf);
    free(ctx->read_buf);
    return err;
  }

  return SYSCORE_SUCCESS;
}

static syscore_error_t pipe_payload_step(void *user_data) {
  pipe_payload_ctx_t *ctx = (pipe_payload_ctx_t *)user_data;
  size_t written = 0;
  size_t read_bytes = 0;

  syscore_error_t err =
      syscore_ipc_write(ctx->wr, ctx->write_buf, ctx->payload_size, &written);
  if (err != SYSCORE_SUCCESS) return err;

  return syscore_ipc_read(ctx->rd, ctx->read_buf, ctx->payload_size, &read_bytes);
}

static void pipe_payload_teardown(void *user_data) {
  pipe_payload_ctx_t *ctx = (pipe_payload_ctx_t *)user_data;
  if (ctx) {
    syscore_ipc_close(ctx->rd);
    syscore_ipc_close(ctx->wr);
    if (ctx->write_buf) free(ctx->write_buf);
    if (ctx->read_buf) free(ctx->read_buf);
  }
}

int main(void) {
  static const size_t payload_sizes[] = {1, 64, 256, 1024};
  static const char *names[] = {
      "IPC Pipe Latency (1 Byte Payload)",
      "IPC Pipe Latency (64 Bytes Payload)",
      "IPC Pipe Latency (256 Bytes Payload)",
      "IPC Pipe Latency (1024 Bytes Payload)"};

  syscore_error_t status = SYSCORE_SUCCESS;

  for (size_t i = 0; i < SYSCORE_ARRAY_SIZE(payload_sizes); i++) {
    pipe_payload_ctx_t ctx;
    ctx.payload_size = payload_sizes[i];

    syscore_benchmark_config_t config;
    config.name = names[i];
    config.warmup_iterations = 500;
    config.measured_iterations = 5000;
    config.setup = pipe_payload_setup;
    config.step = pipe_payload_step;
    config.teardown = pipe_payload_teardown;
    config.user_data = &ctx;

    syscore_error_t err = syscore_benchmark_run(&config, NULL);
    if (err != SYSCORE_SUCCESS) {
      status = err;
    }
  }

  return (status == SYSCORE_SUCCESS) ? 0 : 1;
}
