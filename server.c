#include "gossip.h"


Status
server_init (Server *server)
{
  printf ("server init ...\n");
  int rc;

  server->loop = uv_default_loop ();

  printf ("host init ...\n");
  server->host = calloc (1, sizeof (struct sockaddr_storage));
  if (!server->host)
    goto error;
  uv_ip4_addr (default_host_ip,
               default_host_port,
               (struct sockaddr_in *) server->host);

  printf ("event handle init ...\n");
  rc = event_init (server);
  if (rc < 0)
    goto error;

  printf ("server init OK.\n");
  return G_OK;

error:
  printf ("server init ERR!\n");
  return G_ERR;
}


Status
server_run (Server *server)
{
  printf ("server run ...\n");
  int rc;

  printf ("uv run ...\n");
  rc = uv_run (server->loop, UV_RUN_DEFAULT);
  if (rc < 0)
    goto error;

  printf ("server run OK.\n");
  return G_OK;

error:
  printf ("server run ERR!\n");
  return G_ERR;
}
