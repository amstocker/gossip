#include "gossip.h"
#include "event_map.h"
#include "utils/json.h"


static void libuv_handler (uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
                           const struct sockaddr *addr, unsigned flags);


Status
event_init (Server *server)
{
  Event *event = &server->event;
  int rc;

  rc = event_map_init ();
  if (rc < 0)
    goto error;

  event->json = json_builder_new ();
  if (!event->json)
    goto error;

  rc = uv_udp_init (server->loop, (uv_udp_t *) event);
  if (rc < 0)
    goto error;

  rc = uv_udp_bind ((uv_udp_t *) event, server->host, UV_UDP_REUSEADDR);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  // TODO: handle libuv error codes properly
  return G_ERR;
}


Status
event_start (Server *server)
{
  Event *event = &server->event;
  int rc;

  rc = uv_udp_recv_start ((uv_udp_t *) event, buffer_allocate, libuv_handler);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  return G_ERR;
}


static void
libuv_handler (uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
               const struct sockaddr *addr, unsigned flags)
{
  Event *event = (Event *) req;

  printf ("libuv_handler: nread=%lu, buf=%.*s\n", nread, (int) buf->len, buf->base);

  if (nread == 0)
    goto done;

  if (nread < 0) {
    // TODO: log error
    uv_close ((uv_handle_t *) req, NULL);
    return;
  }

  // TODO: log ...
  // char sender[17] = { 0 };
  // uv_ip4_name((const struct sockaddr_in*) addr, sender, 16);

  if (json_parse_src (event->json, buf->base, nread)
      != JSON_OK)
    goto done;

  JsonVal *val = json_lookup (event->json, "event", 5);
  if (val->type != JSON_STRING)
    goto done;

  EventHandler handler = event_get_handler (val->as_string, val->size);
  if (handler)
    // TODO: handle errors from handler?
    handler (event);

done:
  free (buf->base);
}
