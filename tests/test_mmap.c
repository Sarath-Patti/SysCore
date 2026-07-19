#include "common/errors.h"
#include "memory/mmap.h"
#include "test_helpers.h"
#include <string.h>

#define PAGE_SIZE 4096

int main(void) {
  void *addr = NULL;
  syscore_error_t err = syscore_mmap(
      NULL, PAGE_SIZE, SYSCORE_PROT_READ | SYSCORE_PROT_WRITE,
      SYSCORE_MAP_PRIVATE | SYSCORE_MAP_ANONYMOUS, -1, 0, &addr);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);
  ASSERT_PTR_NE(addr, NULL);

  const char *msg = "MmapAnonTest";
  strcpy((char *)addr, msg);
  ASSERT_TRUE(strcmp((char *)addr, msg) == 0);

  err = syscore_mprotect(addr, PAGE_SIZE, SYSCORE_PROT_READ);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  err = syscore_munmap(addr, PAGE_SIZE);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  printf("test_mmap passed.\n");
  return 0;
}
