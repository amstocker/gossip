#include "gossip.h"
#include "event_handlers.h"
#include "utils/map.h"


#define EVENT_MAX_KEYLEN 64


typedef struct {
  char key[EVENT_MAX_KEYLEN];
  size_t key_size;
  EventHandler handler;
  MapNode node;
} EventKey;


Status event_map_init ();
EventHandler event_get_handler (char *key, size_t key_size);
