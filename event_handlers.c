#include "event_handlers.h"


Status
message_event_handler (Event *event)
{
  printf ("handling MESSAGE event!\n");

  JsonVal *val = json_lookup (event->json, "body", 4);
  if (val->type != JSON_STRING)
    return G_ERR;

  printf ("handling MESSAGE: \"%.*s\"\n", (int) val->size, val->as_string);
  return G_OK;
}


Status
handshake_event_handler (Event *event)
{
  return G_OK;
}
