#include <unistd.h>
#include "gossip.h"


uv_loop_t *loop;
uv_mutex_t thread_lock;
uv_cond_t thread_cond;



/* Gossip Server
 * -------------
 *
 */

char *process_args[2] = { "./gossip-server", NULL };

uv_process_t server;
uv_process_options_t options;

Status
spawn_server ()
{
  options.file = process_args[0];
  options.args = process_args;

  // config process io
  options.stdio_count = 3;
  uv_stdio_container_t child_stdio[3];
  child_stdio[0].flags = UV_IGNORE;
  child_stdio[1].flags = UV_IGNORE;
  child_stdio[2].flags = UV_INHERIT_FD;
  child_stdio[2].data.fd = 2;
  options.stdio = child_stdio;

  debug ("spawning gossip-server");
  int rc = uv_spawn (loop, &server, &options);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  if (rc)
    debug ("error: %s", uv_strerror(rc));
  return G_ERR;
}



/* Thread
 * ------
 *
 */

uv_thread_t thread;
uv_pipe_t server_pipe;
uv_connect_t connect_req;
uv_write_t write_req;

// pipe reconnect
int reconn_attempts = 3;

#define global_buf_len 64000
char global_rbuf[global_buf_len];

void connect_cb (uv_connect_t *req, int status);
void read_alloc_cb (uv_handle_t *_, size_t suggested, uv_buf_t *buf);
void read_cb (uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);
void worker (void *arg);


void
read_alloc_cb (uv_handle_t *_, size_t suggested, uv_buf_t *buf)
{
  buf->base = global_rbuf;
  buf->len = global_buf_len;
}


void
connect_cb (uv_connect_t *req, int status)
{
  int rc = 0;

  debug ("checking connection status");
  if ((rc = status)) {
    if (--reconn_attempts) {
      debug ("attempting reconnect (%d left) ...", reconn_attempts);
      uv_pipe_connect (req, &server_pipe, default_host_pipe, connect_cb);
    }
    goto error;
  }

  debug ("pipe start read");
  rc = uv_read_start ((uv_stream_t *) &server_pipe, read_alloc_cb, read_cb);
  if (rc < 0)
    goto error;

  // let main thread run go
  uv_cond_signal (&thread_cond);
  return;

error:
  if (rc)
    debug ("error: %s", uv_strerror (rc));
}


void
read_cb (uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
  if (nread <= 0)
    return;
  
  debug ("echo: \"%.*s\"", (int) nread, buf->base);
}

  
void
worker (void *arg)
{
  int rc = 0;
  
  debug ("pipe init");
  rc = uv_pipe_init (loop, &server_pipe, 0);
  if (rc < 0)
    goto error;

  debug ("pipe connect: %s", default_host_pipe);
  uv_pipe_connect (&connect_req, &server_pipe, default_host_pipe, connect_cb);
  
  debug ("uv run");
  rc = uv_run (loop, UV_RUN_DEFAULT);
  if (rc < 0)
    goto error;

  return;

error:
  if (rc)
    debug ("error: %s", uv_strerror (rc));
}



/* Command Line Interface
 * ----------------------
 *
 */

Status
cli_init ()
{
  int rc = 0;

  debug ("mutex init");
  rc = uv_mutex_init (&thread_lock);
  if (rc < 0)
    goto error;

  debug ("mutex lock");
  rc = uv_mutex_trylock (&thread_lock);
  if (rc < 0)
    goto error;

  debug ("cond init");
  rc = uv_cond_init (&thread_cond);
  if (rc < 0)
    goto error;

  debug ("thread create");
  rc = uv_thread_create (&thread, worker, NULL);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  if (rc)
    debug ("error: %s", uv_strerror (rc));
  return G_ERR;
}


int
main ()
{
  loop = uv_default_loop ();
  Status stat;

  stat = spawn_server ();
  if (stat) {
    debug ("spawn error");
    goto error;
  }
  
  stat = cli_init ();
  if (stat) {
    debug ("init error");
    goto error;
  }

  debug ("waiting for thread ...");
  uv_cond_wait (&thread_cond, &thread_lock);
  uv_mutex_unlock (&thread_lock);

error:
  debug ("killing server");
  uv_process_kill (&server, SIGINT);
  return 1;
}
