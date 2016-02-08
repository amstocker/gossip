#include <string.h>

#include "gossip.h"
#include "event_handlers.h"
#include "utils/map.h"
#include "utils/json.h"
#include "utils/comparator.h"


static Map *event_map = NULL;

typedef struct {
  char   key[64];
  size_t key_size;
  EventHandler handler;
  MapNode node;
} EventKey;

static EventKey event_keys[] = {
  // defined in event_handlers.h
  EVENT_MAP
};


static void event_cb (uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
                      const struct sockaddr *addr, unsigned flags);


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

  /* Init sockaddr used for UDP.  TODO: should parse either a multiaddr or
   * "ip:port" string ("[ip]:port" for ipv6). */
  struct sockaddr_storage host;
  rc = uv_ip4_addr (server->host_ip, server->host_port,
                    (struct sockaddr_in *) &host);
  if (rc < 0)
    goto error;

  rc = uv_udp_bind ((uv_udp_t *) event, (const struct sockaddr *) &host,
                    UV_UDP_REUSEADDR);
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

  rc = uv_udp_recv_start ((uv_udp_t *) event, buffer_allocate, event_cb);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  return G_ERR;
}


static void
event_cb (uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
          const struct sockaddr *addr, unsigned flags)
{
  Event *event = (Event *) req;
  Server *server = SERVER_FROM_EVENT (event);
  // Time timestamp = time_now ();

  printf ("libuv_handler: nread=%lu, buf=%.*s\n", nread, (int) buf->len, buf->base);

  if (nread == 0)
    goto done;

  if (nread < 0) {
    // TODO: log error
    uv_close ((uv_handle_t *) req, NULL);
    goto reject;
  }

  // TODO: log ...
  // char sender[17] = { 0 };
  // uv_ip4_name((const struct sockaddr_in*) addr, sender, 16);

  if (json_parse_src (event->json, buf->base, nread)
      != JSON_OK)
    goto reject;

  Status stat = G_OK;
  JsonVal *val;

  val = json_lookup (event->json, "event", 5);
  if (val->type != JSON_STRING)
    // reject for invalid event type
    goto reject;

  EventKey *e = map_get (event_map, val->as_string, val->size);
  if (e)
    e->handler (event);

  return;

done:
  // send Ack Response
  free (buf->base);
  return;

reject:
  // send Reject Response
  free (buf->base);
  return;
}
