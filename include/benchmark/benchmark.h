#ifndef SYSCORE_BENCHMARK_H
#define SYSCORE_BENCHMARK_H

#include "common/errors.h"
#include "common/utils.h"
#include <stddef.h>

/**
 * Benchmark callback function signatures.
 *
 * setup_fn: Called once before warm-up/measurement. Allocates/initializes user_data.
 * step_fn: Called repeatedly during warm-up and measurement. The operation being timed.
 * teardown_fn: Called once after measurement finishes to clean up user_data.
 */
typedef syscore_error_t (*syscore_benchmark_setup_fn)(void **user_data);
typedef syscore_error_t (*syscore_benchmark_step_fn)(void *user_data);
typedef void (*syscore_benchmark_teardown_fn)(void *user_data);

/**
 * Configuration structure for a benchmark run.
 */
typedef struct {
  const char *name;                       /**< Name of the benchmark */
  size_t warmup_iterations;               /**< Warm-up iterations (default: 100 if 0) */
  size_t measured_iterations;             /**< Measured iterations (default: 1000 if 0) */
  syscore_benchmark_setup_fn setup;       /**< Optional setup callback (can be NULL) */
  syscore_benchmark_step_fn step;         /**< Measured operation callback (required) */
  syscore_benchmark_teardown_fn teardown; /**< Optional teardown callback (can be NULL) */
  void *user_data;                        /**< Context passed if setup is NULL */
} syscore_benchmark_config_t;

/**
 * Structure containing computed performance statistics for a benchmark run.
 */
typedef struct {
  const char *name;           /**< Benchmark name */
  size_t warmup_iterations;   /**< Number of warm-up iterations executed */
  size_t measured_iterations; /**< Number of measured iterations executed */
  double total_elapsed_sec;   /**< Total elapsed time of measured iterations (seconds) */
  double avg_latency_ns;      /**< Arithmetic mean latency (nanoseconds) */
  double min_latency_ns;      /**< Minimum measured latency (nanoseconds) */
  double max_latency_ns;      /**< Maximum measured latency (nanoseconds) */
  double p50_latency_ns;      /**< 50th percentile / median latency (nanoseconds) */
  double p95_latency_ns;      /**< 95th percentile latency (nanoseconds) */
  double p99_latency_ns;      /**< 99th percentile latency (nanoseconds) */
  double stddev_latency_ns;   /**< Sample standard deviation (nanoseconds) */
  double throughput_ops_sec;  /**< Operations executed per second */
} syscore_benchmark_result_t;

/**
 * Runs a benchmark according to the provided configuration and populates out_result.
 * Also prints the formatted benchmark summary to stdout.
 *
 * @param config Pointer to benchmark configuration.
 * @param out_result Pointer to receive benchmark results (can be NULL).
 * @return SYSCORE_SUCCESS on success, error code on failure.
 */
syscore_error_t syscore_benchmark_run(const syscore_benchmark_config_t *config,
                                       syscore_benchmark_result_t *out_result);

/**
 * Prints benchmark results to stdout in a clean, standardized format.
 *
 * @param result Pointer to populated benchmark result structure.
 */
void syscore_benchmark_print_result(const syscore_benchmark_result_t *result);

/**
 * Formats a nanosecond latency value into a human-readable string with sensible units
 * (ns, us, ms, or s).
 *
 * @param ns Latency in nanoseconds.
 * @param buf Output string buffer.
 * @param size Output buffer capacity.
 */
void syscore_benchmark_format_latency(double ns, char *buf, size_t size);

/**
 * Formats a throughput value (operations per second) into a human-readable string.
 *
 * @param ops_sec Operations per second.
 * @param buf Output string buffer.
 * @param size Output buffer capacity.
 */
void syscore_benchmark_format_throughput(double ops_sec, char *buf, size_t size);

/**
 * Exports a benchmark result structure to a JSON file.
 * If the file exists, the result object is appended to the JSON array.
 * If the file does not exist, a new JSON array file is created.
 *
 * @param result Pointer to populated benchmark result structure.
 * @param filepath Path to the output JSON file.
 * @return SYSCORE_SUCCESS on success, error code on failure.
 */
syscore_error_t syscore_benchmark_export_json(const syscore_benchmark_result_t *result,
                                               const char *filepath);

#endif // SYSCORE_BENCHMARK_H
