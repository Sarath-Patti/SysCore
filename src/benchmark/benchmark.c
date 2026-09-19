#include "benchmark/benchmark.h"
#include "common/logging.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int compare_doubles(const void *a, const void *b) {
  double da = *(const double *)a;
  double db = *(const double *)b;
  if (da < db) return -1;
  if (da > db) return 1;
  return 0;
}

static double calculate_percentile(const double *sorted_samples, size_t count,
                                   double percentile) {
  if (count == 0) return 0.0;
  if (count == 1) return sorted_samples[0];
  if (percentile <= 0.0) return sorted_samples[0];
  if (percentile >= 100.0) return sorted_samples[count - 1];

  double rank = (percentile / 100.0) * (double)(count - 1);
  size_t index = (size_t)rank;
  double fraction = rank - (double)index;

  if (index >= count - 1) {
    return sorted_samples[count - 1];
  }

  return sorted_samples[index] +
         fraction * (sorted_samples[index + 1] - sorted_samples[index]);
}

static uint64_t get_monotonic_time_ns(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

void syscore_benchmark_format_latency(double ns, char *buf, size_t size) {
  if (!buf || size == 0) return;
  if (ns < 0.0) {
    snprintf(buf, size, "0.00 ns");
  } else if (ns < 1000.0) {
    snprintf(buf, size, "%.2f ns", ns);
  } else if (ns < 1000000.0) {
    snprintf(buf, size, "%.2f us", ns / 1000.0);
  } else if (ns < 1000000000.0) {
    snprintf(buf, size, "%.2f ms", ns / 1000000.0);
  } else {
    snprintf(buf, size, "%.2f s", ns / 1000000000.0);
  }
}

void syscore_benchmark_format_throughput(double ops_sec, char *buf, size_t size) {
  if (!buf || size == 0) return;
  if (ops_sec >= 1000000000.0) {
    snprintf(buf, size, "%.2f G ops/sec", ops_sec / 1e9);
  } else if (ops_sec >= 1000000.0) {
    snprintf(buf, size, "%.2f M ops/sec", ops_sec / 1e6);
  } else if (ops_sec >= 1000.0) {
    snprintf(buf, size, "%.2f K ops/sec", ops_sec / 1e3);
  } else {
    snprintf(buf, size, "%.2f ops/sec", ops_sec);
  }
}

void syscore_benchmark_print_result(const syscore_benchmark_result_t *res) {
  if (!res) return;

  char elapsed_str[64];
  char avg_str[64];
  char min_str[64];
  char max_str[64];
  char p50_str[64];
  char p95_str[64];
  char p99_str[64];
  char stddev_str[64];
  char throughput_str[64];

  syscore_benchmark_format_latency(res->total_elapsed_sec * 1e9, elapsed_str,
                                   sizeof(elapsed_str));
  syscore_benchmark_format_latency(res->avg_latency_ns, avg_str, sizeof(avg_str));
  syscore_benchmark_format_latency(res->min_latency_ns, min_str, sizeof(min_str));
  syscore_benchmark_format_latency(res->max_latency_ns, max_str, sizeof(max_str));
  syscore_benchmark_format_latency(res->p50_latency_ns, p50_str, sizeof(p50_str));
  syscore_benchmark_format_latency(res->p95_latency_ns, p95_str, sizeof(p95_str));
  syscore_benchmark_format_latency(res->p99_latency_ns, p99_str, sizeof(p99_str));
  syscore_benchmark_format_latency(res->stddev_latency_ns, stddev_str,
                                   sizeof(stddev_str));
  syscore_benchmark_format_throughput(res->throughput_ops_sec, throughput_str,
                                      sizeof(throughput_str));

  printf("\n================================================================================\n");
  printf("Benchmark: %s\n", res->name ? res->name : "Unnamed");
  printf("================================================================================\n");
  printf("Configuration:\n");
  printf("  Warm-up Iterations : %zu\n", res->warmup_iterations);
  printf("  Measured Iterations: %zu\n", res->measured_iterations);
  printf("--------------------------------------------------------------------------------\n");
  printf("Results:\n");
  printf("  Total Elapsed Time : %s (%.6f s)\n", elapsed_str, res->total_elapsed_sec);
  printf("  Throughput         : %s (%.0f ops/sec)\n", throughput_str,
         res->throughput_ops_sec);
  printf("  Average Latency    : %s\n", avg_str);
  printf("  Min Latency        : %s\n", min_str);
  printf("  Median (p50)       : %s\n", p50_str);
  printf("  p95 Latency        : %s\n", p95_str);
  printf("  p99 Latency        : %s\n", p99_str);
  printf("  Max Latency        : %s\n", max_str);
  printf("  Std Deviation      : %s\n", stddev_str);
  printf("================================================================================\n\n");
}

syscore_error_t syscore_benchmark_run(const syscore_benchmark_config_t *config,
                                       syscore_benchmark_result_t *out_result) {
  if (!config || !config->step) {
    SYSCORE_LOG_ERROR("Benchmark configuration or step function is NULL");
    return SYSCORE_ERROR_INVALID_ARGUMENT;
  }

  size_t warmup_iterations =
      config->warmup_iterations > 0 ? config->warmup_iterations : 100;
  size_t measured_iterations =
      config->measured_iterations > 0 ? config->measured_iterations : 1000;

  void *user_data = config->user_data;
  if (config->setup) {
    syscore_error_t setup_err = config->setup(&user_data);
    if (setup_err != SYSCORE_SUCCESS) {
      SYSCORE_LOG_ERROR("Benchmark setup failed for '%s'",
                        config->name ? config->name : "Unnamed");
      return setup_err;
    }
  }

  /* Warm-up Phase */
  for (size_t i = 0; i < warmup_iterations; i++) {
    syscore_error_t step_err = config->step(user_data);
    if (step_err != SYSCORE_SUCCESS) {
      SYSCORE_LOG_ERROR("Benchmark warm-up failed at iteration %zu", i);
      if (config->teardown) config->teardown(user_data);
      return step_err;
    }
  }

  /* Allocate memory for iteration samples */
  double *samples = (double *)malloc(measured_iterations * sizeof(double));
  if (!samples) {
    SYSCORE_LOG_ERROR("Failed to allocate memory for benchmark samples");
    if (config->teardown) config->teardown(user_data);
    return SYSCORE_ERROR_OUT_OF_MEMORY;
  }

  /* Measurement Phase */
  uint64_t total_start = get_monotonic_time_ns();

  for (size_t i = 0; i < measured_iterations; i++) {
    uint64_t op_start = get_monotonic_time_ns();
    syscore_error_t step_err = config->step(user_data);
    uint64_t op_end = get_monotonic_time_ns();

    if (step_err != SYSCORE_SUCCESS) {
      SYSCORE_LOG_ERROR("Benchmark execution failed at iteration %zu", i);
      free(samples);
      if (config->teardown) config->teardown(user_data);
      return step_err;
    }

    samples[i] = (double)(op_end - op_start);
  }

  uint64_t total_end = get_monotonic_time_ns();

  double total_elapsed_sec = (double)(total_end - total_start) / 1e9;
  if (total_elapsed_sec <= 0.0) {
    total_elapsed_sec = 1e-9;
  }

  /* Calculate mean */
  double sum = 0.0;
  for (size_t i = 0; i < measured_iterations; i++) {
    sum += samples[i];
  }
  double mean = sum / (double)measured_iterations;

  /* Sort samples for percentiles, min, max */
  qsort(samples, measured_iterations, sizeof(double), compare_doubles);

  double min_val = samples[0];
  double max_val = samples[measured_iterations - 1];
  double p50_val = calculate_percentile(samples, measured_iterations, 50.0);
  double p95_val = calculate_percentile(samples, measured_iterations, 95.0);
  double p99_val = calculate_percentile(samples, measured_iterations, 99.0);

  /* Calculate standard deviation */
  double variance_sum = 0.0;
  for (size_t i = 0; i < measured_iterations; i++) {
    double diff = samples[i] - mean;
    variance_sum += diff * diff;
  }
  double stddev = sqrt(variance_sum /
                       (double)(measured_iterations > 1 ? (measured_iterations - 1) : 1));

  double throughput = (double)measured_iterations / total_elapsed_sec;

  /* Teardown Phase */
  if (config->teardown) {
    config->teardown(user_data);
  }

  free(samples);

  syscore_benchmark_result_t res;
  res.name = config->name ? config->name : "Unnamed";
  res.warmup_iterations = warmup_iterations;
  res.measured_iterations = measured_iterations;
  res.total_elapsed_sec = total_elapsed_sec;
  res.avg_latency_ns = mean;
  res.min_latency_ns = min_val;
  res.max_latency_ns = max_val;
  res.p50_latency_ns = p50_val;
  res.p95_latency_ns = p95_val;
  res.p99_latency_ns = p99_val;
  res.stddev_latency_ns = stddev;
  res.throughput_ops_sec = throughput;

  syscore_benchmark_print_result(&res);

  if (out_result) {
    *out_result = res;
  }

  return SYSCORE_SUCCESS;
}
