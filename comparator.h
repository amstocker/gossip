#pragma once
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>


typedef int (*Comparator) (void *lhs, void *rhs, size_t size);

int comparator_int (void*, void*, size_t);
int comparator_float (void*, void*, size_t);
int comparator_double (void*, void*, size_t);
int comparator_bytes (void*, void*, size_t);
int comparator_pointer (void*, void*, size_t);
int comparator_string (void*, void*, size_t);
int comparator_string_ic (void*, void*, size_t);
