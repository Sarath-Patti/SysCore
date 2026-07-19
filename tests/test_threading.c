#include "common/errors.h"
#include "test_helpers.h"
#include "threading/threading.h"

static void *thread_func(void *arg) {
  int *val = (int *)arg;
  *val = 100;
  syscore_thread_sleep_ms(50);
  return arg;
}

int main(void) {
  syscore_thread_t thread;
  int test_val = 0;
  syscore_error_t err =
      syscore_thread_create(&thread, NULL, thread_func, &test_val);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);

  void *retval = NULL;
  err = syscore_thread_join(thread, &retval);
  ASSERT_INT_EQ(err, SYSCORE_SUCCESS);
  ASSERT_PTR_NE(retval, NULL);
  ASSERT_INT_EQ(*(int *)retval, 100);
  ASSERT_INT_EQ(test_val, 100);

  printf("test_threading passed.\n");
  return 0;
}
