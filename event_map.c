#include "event_map.h"


static Map *event_map = NULL;

static EventKey event_keys[] = {
  { "message", 7, message_event_handler }
};


Status
event_map_init ()
{
  event_map = string_map_new (EventKey, node, key);
  if (!event_map)
    return G_ERR;
  
  size_t i = sizeof (event_keys) / sizeof (EventKey);
  while (i--)
    if (map_add (event_map, &event_keys[i], event_keys[i].key_size)
        != MAP_OK)
      goto error;
  
  return G_OK;

error:
  map_free (event_map);
  return G_ERR;
}


EventHandler
event_get_handler (char *key, size_t key_size)
{
  EventKey *e;
  e = map_get (event_map, key, key_size);
  if (e)
    return e->handler;
  else
    return NULL;
}
