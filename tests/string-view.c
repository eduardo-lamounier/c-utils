#include<stdio.h>

#include<inttypes.h>

#ifndef STRING_VIEW_IMPLEMENTATION
#define STRING_VIEW_IMPLEMENTATION
#endif

#include "../string-view.h"

int main(void) {
  const char *hello = "Hello world 20523164214!";

  string_view_t view = str_view_new(hello+12, 11);

  printf("view: |" str_view_FMT "|\n", str_view_ARG(view));

  int32_t n1 = str_view_toint32(view); // Will overflow
  int64_t n2 = str_view_toint64(view);

  printf("to i32: %" PRId32 ", to i64: %" PRId64 ".\n", n1, n2);

  return 0;
}
