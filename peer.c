#include "peer.h"


Peer*
peer_new_from_addr (struct sockaddr *addr)
{
  Peer *p = calloc (1, sizeof(Peer));
  if (!p)
    goto error;

  p->addr = calloc (1, sizeof(struct sockaddr_storage));
  if (!p->addr)
    goto error;
  switch (addr->sa_family) {
    case AF_INET:
      memcpy (p->addr, addr, sizeof(struct sockaddr_in));
      break;
    case AF_INET6:
      memcpy (p->addr, addr, sizeof(struct sockaddr_in6));
  }

  return p;

error:
  if (p)
    free (p->addr);
  return NULL;
}
