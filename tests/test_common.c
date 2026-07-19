#include "common/errors.h"
#include "common/version.h"
#include "test_helpers.h"
#include <string.h>

int main(void) {
  const char *ver = syscore_get_version_string();
  ASSERT_PTR_NE((void *)ver, NULL);
  ASSERT_TRUE(strcmp(ver, "1.0.0") == 0);

  const char *err_str = syscore_error_string(SYSCORE_SUCCESS);
  ASSERT_PTR_NE((void *)err_str, NULL);
  ASSERT_TRUE(strcmp(err_str, "Success") == 0);

  printf("test_common passed.\n");
  return 0;
}
