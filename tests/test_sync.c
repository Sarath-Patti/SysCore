#include "common/errors.h"
#include "sync/sync.h"
#include "test_helpers.h"

int main(void) {
  syscore_mutex_t mutex;
  syscore_error_t err = syscore_mutex_init(&mutex);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  err = syscore_mutex_lock(&mutex);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  err = syscore_mutex_unlock(&mutex);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  err = syscore_mutex_destroy(&mutex);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  syscore_cond_t cond;
  err = syscore_cond_init(&cond);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  err = syscore_cond_destroy(&cond);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  printf("test_sync passed.\n");
  return 0;
}
