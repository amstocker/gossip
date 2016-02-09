#include "event_handlers.h"


Status
message_event_handler (Event *event)
{
  JsonVal *val = json_lookup (event->json, "body", 4);
  if (val->type != JSON_STRING)
    return G_ERR;

  debug ("\"%.*s\"", (int) val->size, val->as_string);
  return G_OK;
}


Status
handshake_event_handler (Event *event)
{
  return G_OK;
}
