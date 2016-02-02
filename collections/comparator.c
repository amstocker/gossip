#include "collections/comparator.h"


#define COMPARATOR(NAME, TYPE) \
  int \
  comparator_##NAME (void *lhs, void *rhs, size_t _) \
  { \
    (void) _; \
    return   *(TYPE*) lhs < *(TYPE*) rhs ? -1 \
           : *(TYPE*) lhs > *(TYPE*) rhs ?  1 \
           : 0; \
  }

#define NUMERIC_COMPARATOR(T) COMPARATOR(T, T)
#define STDINT_COMPARATOR(N) COMPARATOR(N, N##_t)


NUMERIC_COMPARATOR(int)
NUMERIC_COMPARATOR(float)
NUMERIC_COMPARATOR(double)


int
comparator_bytes (void *lhs, void *rhs, size_t size)
{
  int r = 0;
  while (size--) {
    r += *(uint8_t*) lhs - *(uint8_t*) rhs;
    lhs++; rhs++;
  }
  return r;
}


int
comparator_pointer (void *lhs, void *rhs, size_t _)
{
  (void) _;  // unused
  return lhs < rhs ? -1 : lhs > rhs ? 1 : 0;
}


int
comparator_string (void *lhs, void *rhs, size_t size)
{
  return size
         ? strncmp((char*) lhs, (char*) rhs, size)
         : strcmp((char*) lhs, (char*) rhs)
         ;
}


int
comparator_string_ic (void *lhs, void *rhs, size_t size)
{
  return size
         ? strncasecmp((char*) lhs, (char*) rhs, size)
         : strcasecmp((char*) lhs, (char*) rhs)
         ;
}
