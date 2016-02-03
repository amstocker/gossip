#pragma once

#include "uv.h"

#include "utils.h"  /* alloc_buffer */
#include "json.h"
#include "error.h"
#include "server.h"
#include "map.h"


/* EventHandle
 * -----------
 *
 *  Subclass of uv_udp_t that represents a single event at any one
 *  point in time.  For now there exists just one instance as part of
 *  the server.
 *
 */
typedef struct {
  uv_udp_t req;
  JsonBuilder *json;
  GServer *server;
} EventHandle;

typedef struct {
  char key[];
  size_t keylen;
  GStatus (*handler) (EventHandle *);
  MapNode *node;
} EventKey;


Map *event_map;
GStatus event_init (GServer *server);
GStatus event_start (GServer *server);
