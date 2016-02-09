#include "gossip.h"


static void api_newconn (uv_stream_t *api, int status);
static void api_cb (uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);


Status
api_init (Server *server)
{
  Api *api = &server->api;
  int rc;

  uv_fs_t req;
  uv_fs_unlink (server->loop, &req, server->host_pipe, NULL);
  // ignore error if pipe doesn't already exist

  rc = uv_pipe_init (server->loop, (uv_pipe_t *) api, 0);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  return G_ERR;
}


Status
api_start (Server *server)
{
  Api *api = &server->api;
  int rc;

  rc = uv_pipe_bind ((uv_pipe_t *) api, server->host_pipe);
  if (rc < 0)
    goto error;

  rc = uv_listen ((uv_stream_t *) api, server->host_backlog, api_newconn);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  return G_ERR;
}


static void
api_newconn (uv_stream_t *api, int status)
{
  Server *server = SERVER_FROM_API (api);
  int rc;

  if (status) goto error;

  uv_pipe_t *client = malloc (sizeof (uv_pipe_t));
  if (!client)
    goto error;

  rc = uv_pipe_init (server->loop, client, 0);
  if (rc < 0)
    goto error;

  rc = uv_accept (api, (uv_stream_t *) client);
  if (rc < 0)
    goto error;

  rc = uv_read_start ((uv_stream_t *) client, buffer_allocate, api_cb);
  if (rc < 0)
    goto error;

  return;

error:
  if (client) {
    uv_close ((uv_handle_t *) client, NULL);
    free (client);
  }
  return;
}


static void
api_cb (uv_stream_t *client, ssize_t nread, const uv_buf_t *buf)
{
  if (nread == 0)
    goto done;

  if (nread < 0) {
    uv_close ((uv_handle_t *) client, NULL);
    goto reject;
  }

  printf ("got message from api: \"%.*s\"\n", (int) nread, buf->base);


done:
  // done
  goto finally;

reject:
  // reject
  goto finally;

finally:
  free (buf->base);
}
