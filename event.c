#include "gossip.h"
#include "event_handlers.h"
#include "utils/map.h"
#include "utils/json.h"


static Map *event_map = NULL;
static void libuv_handler (uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
                           const struct sockaddr *addr, unsigned flags);


/* Event Handler Map
 * -----------------
 * 
 * The udp callback will match the EventHandler with the value in the "event"
 * field in the json send to the server.
 *
 */
typedef struct {
  char key[64];
  size_t key_size;
  EventHandler handler;
  
  // for internal use by utils/map
  MapNode node;
} EventKey;

static EventKey event_keys[] = {
  { "message", 7, message_event_handler }
};



Status
event_init (Server *server)
{
  Event *event = &server->event;
  int rc;
  
  event_map = string_map_new (EventKey, node, key);
  if (!event_map)
    return G_ERR;
  size_t i = sizeof (event_keys) / sizeof (EventKey);
  while (i--)
    if (map_add (event_map, &event_keys[i], event_keys[i].key_size)
        != MAP_OK)
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

  EventKey *e = map_get (event_map, val->as_string, val->size);
  if (e)
    e->handler (event);

done:
  free (buf->base);
}
