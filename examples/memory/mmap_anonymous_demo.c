#include "common/errors.h"
#include "common/logging.h"
#include "memory/mmap.h"
#include <stdio.h>
#include <string.h>

#define PAGE_SIZE 4096

int main(void) {
  syscore_log_init(SYSCORE_LOG_DEBUG);
  SYSCORE_LOG_INFO("Starting mmap Anonymous Mapping Demo");

  void *addr = NULL;
  syscore_error_t err = syscore_mmap(
      NULL, PAGE_SIZE, SYSCORE_PROT_READ | SYSCORE_PROT_WRITE,
      SYSCORE_MAP_PRIVATE | SYSCORE_MAP_ANONYMOUS, -1, 0, &addr);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("mmap anonymous failed: %s", syscore_error_string(err));
    return 1;
  }

  SYSCORE_LOG_INFO("Mapped anonymous memory successfully at %p", addr);

  // Write some data
  strcpy((char *)addr, "Hello from Anonymous Memory!");
  SYSCORE_LOG_INFO("Written to memory: \"%s\"", (char *)addr);

  // Protect to read-only
  SYSCORE_LOG_INFO("Changing protection to READ-ONLY...");
  err = syscore_mprotect(addr, PAGE_SIZE, SYSCORE_PROT_READ);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("mprotect failed: %s", syscore_error_string(err));
    syscore_munmap(addr, PAGE_SIZE);
    return 1;
  }

  SYSCORE_LOG_INFO("Memory content is still readable: \"%s\"", (char *)addr);

  // Unmap
  SYSCORE_LOG_INFO("Unmapping memory...");
  err = syscore_munmap(addr, PAGE_SIZE);
  if (err != SYSCORE_SUCCESS) {
    SYSCORE_LOG_ERROR("munmap failed: %s", syscore_error_string(err));
    return 1;
  }

  SYSCORE_LOG_INFO("Anonymous mapping demo finished successfully.");
  return 0;
}
