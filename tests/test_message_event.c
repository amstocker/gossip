#include <stdio.h>

#include "deps/include/uv.h"
#include "gossip.h"


const char *JSON = "{\"event\": \"message\", \"body\":\"hello server\"}";

uv_buf_t make_buf ()
{
  uv_buf_t buf;
  buffer_allocate (NULL, 256, &buf);
  memcpy (buf.base, JSON, strlen(JSON) + 1);
  return buf;
}


int main() {

  uv_loop_t *loop = uv_default_loop ();
  uv_udp_t socket;

  int rc;

  rc = uv_udp_init (loop, &socket);
  if (rc < 0)
    goto error;

  struct sockaddr_in addr;
  uv_ip4_addr ("0.0.0.0", 9669, &addr);
  rc = uv_udp_bind (&socket, (const struct sockaddr *) &addr, UV_UDP_REUSEADDR);  
  if (rc < 0)
    goto error;

  uv_udp_send_t req;
  struct sockaddr_in sendaddr;
  uv_ip4_addr ("127.0.0.1", 9670, &sendaddr);

  uv_buf_t buf = make_buf();
  rc = uv_udp_send (&req, &socket, &buf, 1, (const struct sockaddr *) &sendaddr, NULL);
  if (rc < 0)
    goto error;

  printf ("test message event: send success!\n");
  return 0;

error:
  printf ("test message event: ERR!\n");
  return 1;
}
