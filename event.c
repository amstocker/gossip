#include "gossip.h"
#include "utils/json.h"
#include "utils/map.h"


#define EVENT_MAX_KEYLEN 64

// command string to handler func mapping
typedef struct {
  char key[EVENT_MAX_KEYLEN];
  size_t keylen;
  GStatus (*handler) (EventHandle *);
  MapNode *node;
} EventKey;

Map *event_map;



static void libuv_handler (uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
                           const struct sockaddr *addr, unsigned flags);
static GStatus new_message_handler (EventHandle *event);


static const EventKey event_keys[] = {
  { "M", 1, new_message_handler }
};


GStatus
event_init (GServer *server)
{
  EventHandle *handle = &server->event_handle;
  int rc;
  
  if ((handle->json = json_builder_new ())
      != JSON_OK)
    goto error;

  rc = uv_udp_init (server->loop, (uv_udp_t *) handle);
  if (rc < 0)
    goto error;

  rc = uv_udp_bind ((uv_udp_t *) handle,
                    server->host,
                    UV_UDP_REUSEADDR);
  if (rc < 0)
    goto error;

  rc = uv_udp_recv_start ((uv_udp_t *) handle,
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

    EventKey *key = map_get (event_map, val->as_string, val->size);
    key->handler (event);
  }

done:
  free (buf->base);
}


static GStatus
new_message_handler (EventHandle *event)
{
  return G_OK;
}
