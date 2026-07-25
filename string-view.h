/* @author: eduardo-lamounier
 * @date: 25/07/2026 [DD/MM/YYYY]
 *
 * A header-only library containing the implementation of string views.
 *
 * To use it, put before including the file:
 * #define STRING_VIEW_IMPLEMENTATION
*/

#ifndef STRING_VIEW_H
#define STRING_VIEW_H

#include<stdio.h>

#define strview_FMT "%.*s"
#define strview_ARGS(v) (v).length, (v).data

typedef struct {
  const char *data;
  size_t length;
} string_view_t;

// Creates a string view to an empty string literal
#define str_view_empty() str_view_from("")

// Trims spaces from left and right of the view
#define str_view_trim(v) {                                                     \
  str_view_trim_left(v);                                                       \
  str_view_trim_right(v);                                                      \
}

// Creates a string view to the start of a string up to a specified amount of
// characters.
//
// All changes in the original string reflect on the view.
string_view_t str_view_new(const char *str, size_t n);

// Creates a string view to the entirety of a null-terminated string.
//
// All changes in the original string reflect on the view.
string_view_t str_view_from(const char *cstr);

string_view_t str_view_slice(string_view_t view, size_t left, size_t right);

void str_view_trim_left(string_view_t *view);

void str_view_trim_right(string_view_t *view);

#endif


#ifdef STRING_VIEW_IMPLEMENTATION

#include<assert.h>
#include<string.h>

inline static void chop_left(string_view_t *view, size_t n) {
  assert(view != NULL && n <= view->length && view->data != NULL);
  assert(view->data != NULL);
  view->data += n;
  view->length -= n;
}

inline static void chop_right(string_view_t *view, size_t n) {
  assert(view != NULL && n <= view->length && view->data != NULL);
  view->length -= n;
}



string_view_t str_view_new(const char *str, size_t n) {
  assert(str != NULL);

  string_view_t view = {
    .data = str,
    .length = n,
  };

  return view;
}

string_view_t str_view_from(const char *cstr) {
  assert(cstr != NULL);
  return str_view_new(cstr, strlen(cstr));
}

string_view_t str_view_slice(string_view_t view, size_t left, size_t right) {
  assert(left < view.length && right <= view.length);
  assert(view.data != NULL && view.data != NULL);

  string_view_t res = str_view_new(view.data, view.length);

  chop_left(&res, left);
  chop_right(&res, view.length - right);

  return res;
}

void str_view_trim_left(string_view_t *view) {
  assert(view != NULL && view->data != NULL);
  size_t count = 0;
  for(size_t i = 0; i < view->length && view->data[i] == ' ';
      i++, count++);
  chop_left(view, count);
}

void str_view_trim_right(string_view_t *view) {
  assert(view != NULL && view->data != NULL);
  size_t count = 0;
  for(size_t i = view->length; i > 0 && view->data[i-1] == ' ';
      i--, count++);
  chop_right(view, count);
}

#endif
