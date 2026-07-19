#include "common/logging.h"
#include "memory/mmap.h"
#include <stdio.h>
#include <time.h>

#define ITERATIONS 5000
#define PAGE_SIZE 4096

int main(void) {
  syscore_log_init(SYSCORE_LOG_INFO);

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  for (int i = 0; i < ITERATIONS; i++) {
    void *addr = NULL;
    syscore_mmap(NULL, PAGE_SIZE, SYSCORE_PROT_READ | SYSCORE_PROT_WRITE,
                 SYSCORE_MAP_PRIVATE | SYSCORE_MAP_ANONYMOUS, -1, 0, &addr);
    syscore_munmap(addr, PAGE_SIZE);
  }

  clock_gettime(CLOCK_MONOTONIC, &end);

  double elapsed =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
  printf("Mmap/Munmap Page Latency: %.3f us\n", (elapsed / ITERATIONS) * 1e6);

  return 0;
}
