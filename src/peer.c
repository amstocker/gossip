#include "gossip.h"
#include "time.h"


Status
peer_init (Server *server)
{
  server->peers = string_map_new (Peer, node, id);
  if (!server->peers)
    return G_ERR;

  return G_OK;
}


Status
peer_update (Event *event)
{
  Server *server = SERVER_FROM_EVENT (event);
  JsonVal *val;

  val = json_lookup (event->json, "id", 2);
  if (val->type != JSON_STRING)
    goto error;

  /* If the id doesn't exist in our map, just reject this message
   * outright.  New peers can be created with a discover/join event. */
  Peer *peer = map_get (server->peers, val->as_string, val->size);
  if (!peer)
    goto error;

  Time timestamp;
  val = json_lookup (event->json, "timestamp", 9);
  if (val->type != JSON_DOUBLE)
    timestamp = time_now ();
  else
    timestamp = (Time) val->as_double;

  // update last message info and rates ...
  
  printf ("updated peer @ %lf\n", timestamp);

  return G_OK;

error:
  return G_ERR;
}


Status
peer_discover (Server *server, Event *event)
{
  return G_OK;
} 
