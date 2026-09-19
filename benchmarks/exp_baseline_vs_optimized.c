#include "benchmark/benchmark.h"
#include "sync/sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAYLOAD_SIZE 4096

typedef struct {
  syscore_mutex_t mutex;
  char *src_buf;
  char *dst_buf;
} opt_exp_ctx_t;

/* Common setup for both baseline and optimized implementations */
static syscore_error_t opt_exp_setup(void **user_data) {
  opt_exp_ctx_t *ctx = (opt_exp_ctx_t *)*user_data;
  if (!ctx) return SYSCORE_ERROR_INVALID_ARGUMENT;

  ctx->src_buf = (char *)malloc(PAYLOAD_SIZE);
  ctx->dst_buf = (char *)malloc(PAYLOAD_SIZE);
  if (!ctx->src_buf || !ctx->dst_buf) {
    if (ctx->src_buf) free(ctx->src_buf);
    if (ctx->dst_buf) free(ctx->dst_buf);
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  }

  memset(ctx->src_buf, 'X', PAYLOAD_SIZE);
  memset(ctx->dst_buf, 0, PAYLOAD_SIZE);

  syscore_error_t err = syscore_mutex_init(&ctx->mutex);
  if (err != SYSCORE_SUCCESS) {
    free(ctx->src_buf);
    free(ctx->dst_buf);
    return err;
  }

  return SYSCORE_SUCCESS;
}

/* Baseline: Unamortized byte-by-byte lock acquisition (high mutex overhead) */
static syscore_error_t baseline_step(void *user_data) {
  opt_exp_ctx_t *ctx = (opt_exp_ctx_t *)user_data;

  for (size_t i = 0; i < PAYLOAD_SIZE; i++) {
    syscore_mutex_lock(&ctx->mutex);
    ctx->dst_buf[i] = ctx->src_buf[i];
    syscore_mutex_unlock(&ctx->mutex);
  }

  return SYSCORE_SUCCESS;
}

/* Optimized: Block-amortized lock acquisition (single lock acquisition for block transfer) */
static syscore_error_t optimized_step(void *user_data) {
  opt_exp_ctx_t *ctx = (opt_exp_ctx_t *)user_data;

  syscore_mutex_lock(&ctx->mutex);
  memcpy(ctx->dst_buf, ctx->src_buf, PAYLOAD_SIZE);
  syscore_mutex_unlock(&ctx->mutex);

  return SYSCORE_SUCCESS;
}

/* Common teardown for both baseline and optimized implementations */
static void opt_exp_teardown(void *user_data) {
  opt_exp_ctx_t *ctx = (opt_exp_ctx_t *)user_data;
  if (ctx) {
    syscore_mutex_destroy(&ctx->mutex);
    if (ctx->src_buf) free(ctx->src_buf);
    if (ctx->dst_buf) free(ctx->dst_buf);
  }
}

int main(void) {
  syscore_error_t status = SYSCORE_SUCCESS;

  /* 1. Baseline Experiment */
  {
    opt_exp_ctx_t ctx;
    syscore_benchmark_config_t config;
    config.name = "Baseline (Unamortized Byte-by-Byte Locked Transfer)";
    config.warmup_iterations = 200;
    config.measured_iterations = 3000;
    config.setup = opt_exp_setup;
    config.step = baseline_step;
    config.teardown = opt_exp_teardown;
    config.user_data = &ctx;

    syscore_error_t err = syscore_benchmark_run(&config, NULL);
    if (err != SYSCORE_SUCCESS) status = err;
  }

  /* 2. Optimized Experiment */
  {
    opt_exp_ctx_t ctx;
    syscore_benchmark_config_t config;
    config.name = "Optimized (Block-Amortized Locked Transfer)";
    config.warmup_iterations = 200;
    config.measured_iterations = 3000;
    config.setup = opt_exp_setup;
    config.step = optimized_step;
    config.teardown = opt_exp_teardown;
    config.user_data = &ctx;

    syscore_error_t err = syscore_benchmark_run(&config, NULL);
    if (err != SYSCORE_SUCCESS) status = err;
  }

  return (status == SYSCORE_SUCCESS) ? 0 : 1;
}
