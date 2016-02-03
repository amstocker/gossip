#include "utils.h"


uint64_t
time_now ()
{
  static struct timeval tv;

  if (gettimeofday(&tv, NULL))
    return 0;
  return tv.tv_sec * (uint64_t) 1000
         + tv.tv_usec / (uint64_t) 1000;
}


void
alloc_buffer (uv_handle_t *handle, size_t size, uv_buf_t *buf)
{
  buf->base = malloc (size);
  buf->len = size;
}
