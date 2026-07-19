#include "common/logging.h"
#include "ipc/ipc.h"
#include <stdio.h>
#include <time.h>

#define ITERATIONS 10000

int main(void) {
  syscore_log_init(SYSCORE_LOG_INFO);

  syscore_ipc_handle_t rd, wr;
  syscore_ipc_pipe_create(&rd, &wr);

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);

  char val = 'x';
  for (int i = 0; i < ITERATIONS; i++) {
    size_t written = 0;
    size_t read_bytes = 0;
    syscore_ipc_write(wr, &val, 1, &written);
    syscore_ipc_read(rd, &val, 1, &read_bytes);
  }

  clock_gettime(CLOCK_MONOTONIC, &end);

  double elapsed =
      (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
  printf("Pipe Round-Trip Latency: %.3f us\n", (elapsed / ITERATIONS) * 1e6);

  syscore_ipc_close(rd);
  syscore_ipc_close(wr);
  return 0;
}
