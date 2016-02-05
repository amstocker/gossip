#include "event_map.h"


static Map *event_map = NULL;

static const EventKey event_keys[] = {
  { "message", 7, event_message_handler }
};


Status
event_map_init ()
{
  event_map = string_map_new (EventKey, node, key);
  if (!event_map)
    return G_ERR;
  
  printf ("event map init ...\n");
  size_t i = sizeof (event_keys) / sizeof (EventKey);
  printf ("nevents: %lu\n", i);
  while (i--) {
    printf("event map add: %.*s\n", (int) event_keys[i].key_size, event_keys[i].key);
    printf("event map addrs: struct=%p, key=%p, ks=%p, eh=%p, node=%p\n", &event_keys[i], &event_keys[i].key, &event_keys[i].key_size, &event_keys[i].handler, &event_keys[i].node);
    map_add (event_map, (void *) &event_keys[i], event_keys[i].key_size);
  }
  return G_OK;
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
