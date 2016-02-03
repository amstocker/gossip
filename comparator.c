#include "comparator.h"


#define NUMERIC_COMPARATOR(TYPE) \
  int \
  comparator_##TYPE (void *lhs, size_t lsize, void *rhs, size_t rsize) \
  { \
    (void) lsize; \
    (void) rsize; \
    return *(TYPE*) lhs - *(TYPE*) rhs; \
  }

NUMERIC_COMPARATOR(int)
NUMERIC_COMPARATOR(float)
NUMERIC_COMPARATOR(double)


int
comparator_bytes (void *lhs, size_t lsize, void *rhs, size_t rsize)
{
  int r = 0;
  while (rsize--, lsize--) {
    r += *(uint8_t*) lhs - *(uint8_t*) rhs;
    lhs++; rhs++;
  }
  while (rsize--) {
    r -= *(uint8_t*) rhs;
    rhs++;
  }
  return r;
}


int
comparator_pointer (void *lhs, size_t lsize, void *rhs, size_t rsize)
{
  (void) lsize;  // unused
  (void) rsize;
  return lhs < rhs ? -1 : lhs > rhs ? 1 : 0;
}


int
comparator_string (void *lhs, size_t lsize, void *rhs, size_t rsize)
{
  int cmp = strncmp((char*) lhs, (char*) rhs, MIN(lsize, rsize));
  
  if (cmp == 0 && lsize != rsize)
    return lsize - rsize;
  else
    return cmp;
}


#include <stdio.h>
int
comparator_string_ic (void *lhs, size_t lsize, void *rhs, size_t rsize)
{
  int cmp = strncasecmp((char*) lhs, (char*) rhs, MIN(lsize, rsize));
  printf("comparator_string_ic: cmp = %i\n", cmp);

  if (cmp == 0 && lsize != rsize)
    return lsize - rsize;
  else
    return cmp;
}
