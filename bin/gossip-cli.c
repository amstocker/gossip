#include "gossip.h"


char *process_args[2] = { "./gossip-server", NULL };

uv_loop_t *loop;
uv_process_t server;
uv_process_options_t options;
uv_mutex_t write_lock;


Status
spawn_server ()
{
  loop = uv_default_loop ();

  options.file = process_args[0];
  options.args = process_args;

  debug ("spawning gossip-server");
  int rc = uv_spawn (loop, &server, &options);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  debug ("error: %s", uv_strerror(rc));
  return G_ERR;
}


int
main ()
{
  Status stat;

  stat = spawn_server ();

  if (!stat) {
    debug ("spawn OK, killing server");
    uv_process_kill (&server, SIGINT);
  }

  return 0;
}
