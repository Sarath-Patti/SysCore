#include "benchmark/benchmark.h"
#include "test_helpers.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int setup_called;
  int step_called;
  int teardown_called;
} test_ctx_t;

static syscore_error_t test_setup(void **user_data) {
  test_ctx_t *ctx = (test_ctx_t *)malloc(sizeof(test_ctx_t));
  if (!ctx) return SYSCORE_ERROR_OUT_OF_MEMORY;
  ctx->setup_called = 1;
  ctx->step_called = 0;
  ctx->teardown_called = 0;
  *user_data = ctx;
  return SYSCORE_SUCCESS;
}

static syscore_error_t test_step(void *user_data) {
  test_ctx_t *ctx = (test_ctx_t *)user_data;
  if (ctx) {
    ctx->step_called++;
  }
  return SYSCORE_SUCCESS;
}

static void test_teardown(void *user_data) {
  test_ctx_t *ctx = (test_ctx_t *)user_data;
  if (ctx) {
    ctx->teardown_called = 1;
    free(ctx);
  }
}

int main(void) {
  /* Test 1: Invalid Argument */
  syscore_error_t err = syscore_benchmark_run(NULL, NULL);
  ASSERT_INT_EQ(err, SYSCORE_ERROR_INVALID_ARGUMENT);

  syscore_benchmark_config_t invalid_cfg;
  invalid_cfg.name = "Invalid";
  invalid_cfg.warmup_iterations = 10;
  invalid_cfg.measured_iterations = 10;
  invalid_cfg.setup = NULL;
  invalid_cfg.step = NULL;
  invalid_cfg.teardown = NULL;
  invalid_cfg.user_data = NULL;
  err = syscore_benchmark_run(&invalid_cfg, NULL);
  ASSERT_INT_EQ(err, SYSCORE_ERROR_INVALID_ARGUMENT);

  /* Test 2: Valid Benchmark Run and Statistics Invariants */
  syscore_benchmark_config_t config;
  config.name = "Framework Unit Test";
  config.warmup_iterations = 50;
  config.measured_iterations = 500;
  config.setup = test_setup;
  config.step = test_step;
  config.teardown = test_teardown;
  config.user_data = NULL;

  syscore_benchmark_result_t result;
  err = syscore_benchmark_run(&config, &result);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  /* Verify iteration counts */
  ASSERT_INT_EQ((int)result.warmup_iterations, 50);
  ASSERT_INT_EQ((int)result.measured_iterations, 500);

  /* Verify statistical invariants */
  ASSERT_TRUE(result.min_latency_ns <= result.avg_latency_ns);
  ASSERT_TRUE(result.avg_latency_ns <= result.max_latency_ns);
  ASSERT_TRUE(result.min_latency_ns <= result.p50_latency_ns);
  ASSERT_TRUE(result.p50_latency_ns <= result.p95_latency_ns);
  ASSERT_TRUE(result.p95_latency_ns <= result.p99_latency_ns);
  ASSERT_TRUE(result.p99_latency_ns <= result.max_latency_ns);
  ASSERT_TRUE(result.throughput_ops_sec > 0.0);
  ASSERT_TRUE(result.total_elapsed_sec > 0.0);

  /* Test formatting functions */
  char buf[64];
  syscore_benchmark_format_latency(500.0, buf, sizeof(buf));
  ASSERT_TRUE(buf[0] != '\0');

  syscore_benchmark_format_throughput(1000000.0, buf, sizeof(buf));
  ASSERT_TRUE(buf[0] != '\0');

  printf("test_benchmark: All tests passed.\n");
  return 0;
}
