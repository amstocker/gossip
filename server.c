#include "gossip.h"


Status
server_init (Server *server)
{
  server->loop = uv_default_loop ();

  // do this for now instead of loading config ...
  server->host_username = "Andrew";
  
  server->host_ip = (char *) default_host_ip;
  server->host_port = default_host_port;
  server->retries = default_retries;
  server->retry_wait = default_retry_wait;
  server->ratelim = default_ratelim;
  server->ratelim_burst = default_ratelim_burst;
  server->ratelim_window = default_ratelim_window;
  server->ratelim_burst_window = default_ratelim_burst_window;

  
  Status stat = G_OK;

  stat = peer_init (server);
  if (stat)
    goto error;

  stat = event_init (server);
  if (stat)
    goto error;

  return G_OK;

error:
  printf ("server init ERR!\n");
  return stat;
}


Status
server_run (Server *server)
{
  Status stat = G_OK;
  int rc = 0;

  stat = event_start (server);
  if (stat)
    goto error;

  rc = uv_run (server->loop, UV_RUN_DEFAULT);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  printf ("server run ERR!\n");
  return G_ERR;
}
