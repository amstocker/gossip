#include "gossip.h"


void
buffer_allocate (uv_handle_t *handle, size_t suggested, uv_buf_t *buf)
{
  buf->base = malloc (suggested);
  buf->len = suggested;
}
