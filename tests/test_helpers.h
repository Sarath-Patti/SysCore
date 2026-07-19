#ifndef SYSCORE_TEST_HELPERS_H
#define SYSCORE_TEST_HELPERS_H

#include <stdio.h>
#include <stdlib.h>

#define ASSERT_TRUE(cond)                                                      \
  do {                                                                         \
    if (!(cond)) {                                                             \
      fprintf(stderr, "%s:%d: Assertion '%s' failed.\n", __FILE__, __LINE__,   \
              #cond);                                                          \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define ASSERT_INT_EQ(actual, expected)                                        \
  do {                                                                         \
    int a = (actual);                                                          \
    int e = (expected);                                                        \
    if (a != e) {                                                              \
      fprintf(stderr, "%s:%d: Assertion failed: expected %d, got %d.\n",       \
              __FILE__, __LINE__, e, a);                                       \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define ASSERT_PTR_NE(ptr, val)                                                \
  do {                                                                         \
    void *p = (ptr);                                                           \
    if (p == (val)) {                                                          \
      fprintf(stderr, "%s:%d: Assertion failed: pointer is %p.\n", __FILE__,   \
              __LINE__, p);                                                    \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#endif // SYSCORE_TEST_HELPERS_H
