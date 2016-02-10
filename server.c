#include "gossip.h"


Status
server_init (Server *server)
{
  server->loop = uv_default_loop ();

  // do this for now instead of loading config ...
  server->host_username = "Andrew";
  
  server->host_ip = (char *) default_host_ip;
  server->host_port = default_host_port;
  server->host_pipe = (char *) default_host_pipe;
  server->host_backlog = default_host_backlog;
  server->retries = default_retries;
  server->retry_wait = default_retry_wait;
  server->ratelim = default_ratelim;
  server->ratelim_burst = default_ratelim_burst;
  server->ratelim_window = default_ratelim_window;
  server->ratelim_burst_window = default_ratelim_burst_window;

  
  Status stat = G_OK;

  debug ("peer init");
  stat = peer_init (server);
  if (stat)
    goto error;

  debug ("event init");
  stat = event_init (server);
  if (stat)
    goto error;

  debug ("api init");
  stat = api_init (server);
  if (stat)
    goto error;

  return G_OK;

error:
  debug ("error");
  return stat;
}


Status
server_run (Server *server)
{
  Status stat = G_OK;
  int rc = 0;

  debug ("event start");
  stat = event_start (server);
  if (stat)
    goto error;

  debug ("api start");
  stat = api_start (server);
  if (stat)
    goto error;

  debug ("uv run");
  rc = uv_run (server->loop, UV_RUN_DEFAULT);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  debug ("error");
  return G_ERR;
}


void
server_fatal (Server *server, int status)
{
  debug ("exit with status %i", status);
  exit (status);
}
