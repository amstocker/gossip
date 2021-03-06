#include "gossip.h"


Server server;

int main ()
{
  Status stat;

  debug ("init");
  stat = server_init (&server);
  if (stat)
    goto error;

  debug ("run");
  stat = server_run (&server);
  if (stat)
    goto error;

  debug ("ok");
  return G_OK;

error:
  debug ("error");
  return G_ERR;
}
