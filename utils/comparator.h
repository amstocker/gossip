#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>

#include "utils/macros.h"


typedef int (*Comparator) (void *lhs, size_t lsize, void *rhs, size_t rsize);

int comparator_int (void*, size_t, void*, size_t);
int comparator_float (void*, size_t, void*, size_t);
int comparator_double (void*, size_t, void*, size_t);
int comparator_bytes (void*, size_t, void*, size_t);
int comparator_pointer (void*, size_t, void*, size_t);
int comparator_string (void*, size_t, void*, size_t);
int comparator_string_ic (void*, size_t, void*, size_t);
