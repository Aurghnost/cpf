#ifndef _UNITTEST_H_INCLUDED
#define _UNITTEST_H_INCLUDED

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>

// Flags characterizing a unit test
#define UNITTEST_TODO (1 << 0) // To write unittest
#define UNITTEST_DONE (1 << 1) // Written unittest

//! All unittesting necessary global variables
typedef struct __unittest {
  unsigned int passed;
  unsigned int failed;
  unsigned int todo;
  unsigned int local_fails;
  int stdout_back;
  int std_tmp;
} __unittest;

#define unittest_init() __unittest __ut = {0}

#define unittest(tests)                                                        \
  int main(int argc, char *argv[]) {                                           \
    printf("Running test file \033[93m'%s'\033[0m\n", __FILE__);               \
    tests \
    return EXIT_SUCCESS;                                                       \
  }

#define unittest_test(testname, code, flags)                                   \
  {                                                                            \
    printf("\033[94m" testname "\033[0m\n");                                   \
    if (flags & UNITTEST_TODO) {                                               \
      printf("\t\033[0;35mThis test has not yet been written.\033[0m\n");      \
    } else {                                                                   \
      __ut.local_fails = 0;                                                    \
      fflush(stdout);                                                          \
      __ut.stdout_back = dup(1);                                               \
      __ut.std_tmp = open("/dev/null", O_WRONLY);                              \
      dup2(__ut.std_tmp, 1);                                                   \
      close(__ut.std_tmp);                                                     \
      code \
	fflush(stdout);                                                     \
      dup2(__ut.stdout_back, 1);                                               \
      close(__ut.stdout_back);                                                 \
      if (__ut.local_fails) {                                                  \
        printf("\t| \033[91mFAILED !\033[0m\n");                               \
        __ut.failed++;                                                         \
      } else {                                                                 \
        printf("\t| \033[92mPASSED !\033[0m\n");                               \
        __ut.passed++;                                                         \
      }                                                                        \
    }                                                                          \
  }

#define unittest_fail(__format, args...)                                       \
  {                                                                            \
    __ut.local_fails++;                                                        \
    fprintf(stderr,                                                            \
            "\t| l.%d: \033[91m"__format                                       \
            "\033[0m\n",                                                       \
            __LINE__, ##args);                                                 \
  }

#define unittest_assert(expr, __format, args...)                               \
  if (!(expr))                                                                 \
  unittest_fail(__format, ##args)

#endif
