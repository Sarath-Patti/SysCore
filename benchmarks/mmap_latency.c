#include "benchmark/benchmark.h"
#include "memory/mmap.h"
#include <stdlib.h>

#define MMAP_BENCH_PAGE_SIZE 4096

static syscore_error_t mmap_step(void *user_data) {
  SYSCORE_UNUSED(user_data);
  void *addr = NULL;
  syscore_error_t err = syscore_mmap(NULL, MMAP_BENCH_PAGE_SIZE,
                                      SYSCORE_PROT_READ | SYSCORE_PROT_WRITE,
                                      SYSCORE_MAP_PRIVATE | SYSCORE_MAP_ANONYMOUS,
                                      -1, 0, &addr);
  if (err != SYSCORE_SUCCESS) return err;
  return syscore_munmap(addr, MMAP_BENCH_PAGE_SIZE);
}

int main(void) {
  syscore_benchmark_config_t config;
  config.name = "Memory Mapping (Mmap/Munmap) Page Latency";
  config.warmup_iterations = 500;
  config.measured_iterations = 5000;
  config.setup = NULL;
  config.step = mmap_step;
  config.teardown = NULL;
  config.user_data = NULL;

  return (syscore_benchmark_run(&config, NULL) == SYSCORE_SUCCESS) ? 0 : 1;
}
