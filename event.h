#pragma once

#include "uv.h"

#include "utils.h"  /* alloc_buffer */
#include "json.h"
#include "error.h"
#include "server.h"
#include "map.h"

#define EVENT_MAX_KEYLEN 64


// command string to handler func mapping
typedef struct {
  char key[EVENT_MAX_KEYLEN];
  size_t keylen;
  GStatus (*handler) (EventHandle *);
  MapNode *node;
} EventKey;

Map *event_map;


GStatus event_init (GServer *server);
GStatus event_start (GServer *server);
