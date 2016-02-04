#include <string.h>

#include "gossip.h"


Peer*
peer_new ()
{
  Peer *p = calloc (1, sizeof (Peer));
  if (!p)
    return NULL;

  uuid_generate_time_safe (p->id);
  return p;
}


GStatus
peer_set_name (Peer *p, const char *buf, size_t len)
{
  if (len > PEER_MAX_NAME_LEN)
    return G_ERR;

  memcpy(p->name, buf, len);
  return G_OK;
}


GStatus
peer_set_addr (Peer *p, const struct sockaddr *addr)
{
  if (p->addr)
    free (p->addr);
  
  struct sockaddr *tmp = calloc (1, sizeof(struct sockaddr_storage));
  if (!tmp)
    return G_ERR;
  p->addr = tmp;

  switch (addr->sa_family) {
    case AF_INET:
      memcpy (p->addr, addr, sizeof(struct sockaddr_in));
      break;
    case AF_INET6:
      memcpy (p->addr, addr, sizeof(struct sockaddr_in6));
  }

  return G_OK;
}
