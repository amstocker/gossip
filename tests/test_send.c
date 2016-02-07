#include "test_send.h"


static
uv_buf_t make_buf (const char *msg, size_t len)
{
  uv_buf_t buf;
  buffer_allocate (NULL, 4096, &buf);
  memset (buf.base, 0, 4096);
  memcpy (buf.base, msg, len);
  return buf;
}


Status
send_message (const char *ip, short port, const char *msg, size_t len)
{
  uv_loop_t *loop = uv_default_loop ();
  uv_udp_t socket;
  int rc;

  rc = uv_udp_init (loop, &socket);
  if (rc < 0)
    goto error;

  struct sockaddr_in addr;
  uv_ip4_addr ("0.0.0.0", 9600, &addr);
  rc = uv_udp_bind (&socket, (const struct sockaddr *) &addr, UV_UDP_REUSEADDR);  
  if (rc < 0)
    goto error;

  uv_udp_send_t req;
  struct sockaddr_in sendaddr;
  uv_ip4_addr (ip, port, &sendaddr);

  uv_buf_t buf = make_buf (msg, len);
  rc = uv_udp_send (&req, &socket, &buf, 1, (const struct sockaddr *) &sendaddr, NULL);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  return G_ERR;
}
