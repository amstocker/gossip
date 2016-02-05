#include "gossip.h"


int main ()
{
  Server server;
  Status stat;

  printf ("gossip init ...\n");

  stat = server_init (&server);
  if (stat)
    goto error;

  stat = server_run (&server);
  if (stat)
    goto error;

  return G_OK;

error:
  printf ("gossip ERR!\n");
  return G_ERR;
}
