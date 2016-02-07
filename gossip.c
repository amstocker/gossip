#include "gossip.h"


int main ()
{
  Server server;
  Status stat;

  stat = server_init (&server);
  if (stat)
    goto error;

  stat = server_run (&server);
  if (stat)
    goto error;

  return G_OK;

error:
  return G_ERR;
}
