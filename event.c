#include "event.h"


static void libuv_handler (uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
                           const struct sockaddr *addr, unsigned flags);

static EventKey[] event_keys = {
  
};


GStatus
event_init (GServer *server)
{
  if ((server->event_handle->json = json_builder_new ())
      != JSON_OK)
    return G_ERR;
  
  if (uv_udp_init (server->loop, (uv_udp_t *) &server->event_handle)
      != 0)
    return G_ERR;

  // for getting server ptr from callback
  server.event_handle.server = server;
  return G_OK;
}


GStatus
event_start (GServer *server)
{
  int rc;
  rc = uv_udp_bind ((uv_udp_t *) &server->event_handle,
                    server->host,
                    UV_UDP_REUSEADDR);
  if (rc < 0)
    goto error;

  rc = uv_udp_recv_start ((uv_udp_t *) &server->event_handle,
                          alloc_buffer,
                          libuv_handler);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  // TODO: handle libuv error codes properly
  return G_ERR;
}


static void
libuv_handler (uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
               const struct sockaddr *addr, unsigned flags)
{
  EventHandle *event = (EventHandle *) req;

  if (nread < 0) {
    // TODO: log error
    uv_close ((uv_handle_t *) req, NULL);
    return;
  }

  // TODO: log ...
  // char sender[17] = { 0 };
  // uv_ip4_name((const struct sockaddr_in*) addr, sender, 16);

  if (nread > 0) {
    
    // if json is not parsable just abandon packet
    if (json_parse_src (event->json, buf->base, nread)
        != JSON_OK)
      goto done;

    // get event type
    JsonVal *val = json_lookup (event->json, "event", 5);
    if (val->type != JSON_STRING)
      goto done;

    EventKey *key = map_get (event_map, 
  }

done:
  free (buf->base);
}
