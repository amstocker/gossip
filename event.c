#include "gossip.h"
#include "event_handlers.h"


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


static void event_alloc_cb (uv_handle_t *req, size_t suggested, uv_buf_t *buf);
static void event_cb (uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
                      const struct sockaddr *addr, unsigned flags);


Status
event_init (Server *server)
{
  Event *event = &server->event;
  int rc;

  event->reusable_base = NULL;
  event->base_alloc = false;

  debug ("event map init");  
  event_map = string_map_new (EventKey, node, key);
  if (!event_map)
    return G_ERR;
  
  size_t i = sizeof (event_keys) / sizeof (EventKey);
  while (i--)
    if (map_add (event_map, &event_keys[i], event_keys[i].key_size)
        != MAP_OK)
      goto error;
  
  debug ("json parser init");
  event->json = json_parser_new ();
  if (!event->json)
    goto error;

  debug ("udp init");
  rc = uv_udp_init (server->loop, (uv_udp_t *) event);
  if (rc < 0)
    goto error;

  /* Init sockaddr used for UDP.  TODO: should parse either a multiaddr or
   * "ip:port" string ("[ip]:port" for ipv6). */
  debug ("udp host init: %s:%d", server->host_ip, server->host_port);
  struct sockaddr_storage host;
  rc = uv_ip4_addr (server->host_ip, server->host_port,
                    (struct sockaddr_in *) &host);
  if (rc < 0)
    goto error;

  debug ("udp bind: %s:%d", server->host_ip, server->host_port);
  rc = uv_udp_bind ((uv_udp_t *) event, (const struct sockaddr *) &host,
                    UV_UDP_REUSEADDR);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  debug ("error");
  // TODO: handle libuv error codes properly
  return G_ERR;
}


Status
event_start (Server *server)
{
  Event *event = &server->event;
  int rc;

  debug ("udp recv start");
  rc = uv_udp_recv_start ((uv_udp_t *) event, event_alloc_cb, event_cb);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  return G_ERR;
}


static void
event_alloc_cb (uv_handle_t *req, size_t suggested, uv_buf_t *buf)
{
  Event *event = (Event *) req;
  if (event->base_alloc) {
    debug ("buffer double allocated");
    exit (0);
  }
  
  debug ("allocating buffer");
  if (!event->reusable_base)
    event->reusable_base = malloc (suggested);
  
  // memset (event->reusable_base, 0, suggested);
  buf->base = event->reusable_base;
  buf->len = suggested;

  event->base_alloc = true;
}


static void
event_cb (uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
          const struct sockaddr *addr, unsigned flags)
{
  Event *event = (Event *) req;
  Server *server = SERVER_FROM_EVENT (event);
  // Time timestamp = time_now ();

  debug ("\"%.*s\"", (int) nread, buf->base);

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

  debug ("parsing json");
  if (json_parse_src (event->json, buf->base, nread)
      != JSON_OK)
    goto reject;

  Status stat = G_OK;
  JsonVal *val;

  val = json_lookup (event->json, "event", 5);
  if (val->type != JSON_STRING)
    // reject for invalid event type
    goto reject;

  debug ("handling event: %.*s", (int) val->size, val->as_string);
  EventKey *e = map_get (event_map, val->as_string, val->size);
  if (e)
    e->handler (event);

done:
  debug ("done");
  // send Ack Response
  goto finally;

reject:
  debug ("reject");
  // send Reject Response
  goto finally;

finally:
  event->base_alloc = false;
  return;
}
