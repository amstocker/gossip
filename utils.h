#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#include "uv.h"


// macros
#define MIN(x, y) (y ^ ((x ^ y) & -(x < y)))
#define MAX(x, y) (x ^ ((x ^ y) & -(x < y)))

#define container_of(P, T, M) ((T*) ((size_t) P - offsetof(T, M)))


// timestamp in millis
uint64_t time_now ();

// libuv handler for buffer allocation
void alloc_buffer (uv_handle_t *handle, size_t size, uv_buf_t *buf);
