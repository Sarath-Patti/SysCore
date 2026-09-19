#include "benchmark/benchmark.h"
#include "ipc/message_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MQ_BENCH_NAME "/syscore_ipc_mq_bench"

typedef struct {
  syscore_mq_handle_t mq;
  size_t msg_size;
  char *send_buf;
  char *recv_buf;
} mq_bench_ctx_t;

static syscore_error_t mq_bench_setup(void **user_data) {
  mq_bench_ctx_t *ctx = (mq_bench_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  ctx->send_buf = (char *)malloc(ctx->msg_size);
  ctx->recv_buf = (char *)malloc(ctx->msg_size);
  if (!ctx->send_buf || !ctx->recv_buf) {
    if (ctx->send_buf) free(ctx->send_buf);
    if (ctx->recv_buf) free(ctx->recv_buf);
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  }
  memset(ctx->send_buf, 'M', ctx->msg_size);

  syscore_mq_attr_t attr;
  attr.flags = 0;
  attr.max_msgs = 10;
  attr.msg_size = (long)ctx->msg_size;
  attr.cur_msgs = 0;

  syscore_error_t err = syscore_mq_open(MQ_BENCH_NAME, 2, 0, 0666, &attr, &ctx->mq);
  if (err != SYSCORE_SUCCESS) {
    free(ctx->send_buf);
    free(ctx->recv_buf);
    return err;
  }

  return SYSCORE_SUCCESS;
}

static syscore_error_t mq_bench_step(void *user_data) {
  mq_bench_ctx_t *ctx = (mq_bench_ctx_t *)user_data;
  unsigned int prio = 1;
  size_t read_bytes = 0;

  syscore_error_t err = syscore_mq_send(ctx->mq, ctx->send_buf, ctx->msg_size, prio);
  if (err != SYSCORE_SUCCESS) return err;

  return syscore_mq_receive(ctx->mq, ctx->recv_buf, ctx->msg_size, NULL, &read_bytes);
}

static void mq_bench_teardown(void *user_data) {
  mq_bench_ctx_t *ctx = (mq_bench_ctx_t *)user_data;
  if (ctx) {
    syscore_mq_close(ctx->mq);
    syscore_mq_unlink(MQ_BENCH_NAME);
    if (ctx->send_buf) free(ctx->send_buf);
    if (ctx->recv_buf) free(ctx->recv_buf);
  }
}

int main(void) {
  static const size_t msg_sizes[] = {64, 256};
  static const char *names[] = {
      "Message Queue Latency (64 Bytes Payload)",
      "Message Queue Latency (256 Bytes Payload)"};

  syscore_error_t status = SYSCORE_SUCCESS;

  for (size_t i = 0; i < SYSCORE_ARRAY_SIZE(msg_sizes); i++) {
    mq_bench_ctx_t ctx;
    ctx.msg_size = msg_sizes[i];

    syscore_benchmark_config_t config;
    config.name = names[i];
    config.warmup_iterations = 200;
    config.measured_iterations = 2000;
    config.setup = mq_bench_setup;
    config.step = mq_bench_step;
    config.teardown = mq_bench_teardown;
    config.user_data = &ctx;

    syscore_error_t err = syscore_benchmark_run(&config, NULL);
    if (err != SYSCORE_SUCCESS) {
      status = err;
    }
  }

  return (status == SYSCORE_SUCCESS) ? 0 : 1;
}
