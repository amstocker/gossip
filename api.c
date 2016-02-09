#include "gossip.h"


static void api_newconn (uv_stream_t *req, int status);
static void api_alloc_cb (uv_handle_t *req, size_t suggested, uv_buf_t *buf);
static void api_write_cb (uv_write_t *req, int status);
static void api_cb (uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);


Status
api_init (Server *server)
{
  Api *api = &server->api;
  int rc;

  api->reusable_base = NULL;

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
api_newconn (uv_stream_t *req, int status)
{
  Api *api = (Api *) req;
  Server *server = SERVER_FROM_API (api);
  int rc;

  rc = uv_pipe_init (server->loop, &api->client, 0);
  if (rc < 0)
    goto error;

  uv_stream_t *client = (uv_stream_t *) &api->client;
  rc = uv_accept ((uv_stream_t *) api, client);
  if (rc < 0)
    goto error;

  rc = uv_read_start (client, api_alloc_cb, api_cb);
  if (rc < 0)
    goto error;

  return;

error:
  if (client) {
    uv_close ((uv_handle_t *) client, NULL);
  }
  return;
}


static void
api_alloc_cb (uv_handle_t *req, size_t suggested, uv_buf_t *buf)
{
  Api *api = API_FROM_CLIENT (req);
  if (!api->reusable_base)
    api->reusable_base = malloc (suggested);

  // memset (api->reusable_base, 0, suggested);
  buf->base = api->reusable_base;
  buf->len = suggested;
}


void
api_write_cb (uv_write_t *req, int status)
{
  if (status)
    printf ("api write ERR!\n");
  else
    printf ("api write OK.\n");
  
  free (req);
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

  uv_write_t *req = malloc (sizeof (uv_write_t));
  uv_buf_t wbuf = uv_buf_init (buf->base, nread);
  
  uv_write (req, client, &wbuf, 1, api_write_cb);


done:
  // done
  goto finally;

reject:
  // reject
  goto finally;

finally:
  return;
}
