#include "gossip.h"


Status
server_init (Server *server)
{
  server->loop = uv_default_loop ();
  int rc;

  server->host = calloc (1, sizeof (struct sockaddr_storage));
  if (!server->host)
    goto error;
  uv_ip4_addr (default_host_ip,
               default_host_port,
               (struct sockaddr_in *) server->host);

  rc = event_init (server);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  printf ("server init ERR!\n");
  return G_ERR;
}


Status
server_run (Server *server)
{
  int rc;

  rc = event_start (server);
  if (rc < 0)
    goto error;

  rc = uv_run (server->loop, UV_RUN_DEFAULT);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  printf ("server run ERR!\n");
  return G_ERR;
}
