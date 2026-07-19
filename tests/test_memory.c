#include "common/errors.h"
#include "memory/shared_memory.h"
#include "test_helpers.h"
#include <string.h>

#define SHM_TEST "/sc_shm_test"
#define SHM_SIZE 128

int main(void) {
  syscore_shm_destroy(SHM_TEST);

  syscore_shm_handle_t fd;
  syscore_error_t err = syscore_shm_create(SHM_TEST, SHM_SIZE, 0666, &fd);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  void *addr = NULL;
  err = syscore_shm_map(fd, SHM_SIZE, 1, &addr);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);
  ASSERT_PTR_NE(addr, NULL);

  const char *msg = "SharedMemoryTest";
  strcpy((char *)addr, msg);

  err = syscore_shm_unmap(addr, SHM_SIZE);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  syscore_shm_close(fd);
  err = syscore_shm_destroy(SHM_TEST);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  printf("test_memory passed.\n");
  return 0;
}
