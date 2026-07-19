#include "common/errors.h"
#include "sync/semaphore.h"
#include "test_helpers.h"

#define SEM_TEST_NAME "/sc_sem_test"

int main(void) {
  // Test unnamed semaphore
  syscore_sem_t sem;
  syscore_error_t err = syscore_sem_init(&sem, 0, 1);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  err = syscore_sem_wait(&sem);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  err = syscore_sem_post(&sem);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  err = syscore_sem_destroy(&sem);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  // Test named semaphore
  syscore_sem_unlink(SEM_TEST_NAME);

  syscore_sem_t nsem;
  err = syscore_sem_open(SEM_TEST_NAME, 0666, 1, &nsem);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  err = syscore_sem_wait(&nsem);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  err = syscore_sem_post(&nsem);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  err = syscore_sem_close(&nsem);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  err = syscore_sem_unlink(SEM_TEST_NAME);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  printf("test_semaphore passed.\n");
  return 0;
}
