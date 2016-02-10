#include "gossip.h"


static void api_newconn (uv_stream_t *req, int status);
static void api_alloc_cb (uv_handle_t *req, size_t suggested, uv_buf_t *buf);
static void api_cb (uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);


Status
api_init (Server *server)
{
  Api *api = &server->api;
  int rc = 0;

  api->reusable_base = NULL;
  api->base_alloc = false;

  debug ("pipe unlink: %s", server->host_pipe);
  uv_fs_t req;
  uv_fs_unlink (server->loop, &req, server->host_pipe, NULL);
  // ignore error if pipe doesn't already exist

  debug ("pipe init");
  rc = uv_pipe_init (server->loop, (uv_pipe_t *) api, 0);
  if (rc < 0)
    goto error;

  debug ("pipe bind: %s", server->host_pipe);
  rc = uv_pipe_bind ((uv_pipe_t *) api, server->host_pipe);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  if (rc)
    debug ("error: %s", uv_strerror (rc));
  return G_ERR;
}


Status
api_start (Server *server)
{
  Api *api = &server->api;
  int rc = 0;

  debug ("pipe listen");
  rc = uv_listen ((uv_stream_t *) api, server->host_backlog, api_newconn);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  if (rc)
    debug ("error: %s", uv_strerror (rc));
  return G_ERR;
}


Status
api_send (Server *server, uv_buf_t *buf, uv_write_cb callback)
{
  uv_pipe_t *client = (uv_pipe_t *) &server->api.client;
  int rc = 0;

  if (!uv_is_active ((uv_handle_t *) client))
    goto error;

  uv_write_t req;
  rc = uv_write (&req, (uv_stream_t *) client, buf, 1, callback);
  if (rc < 0)
    goto error;

  return G_OK;

error:
  if (rc)
    debug ("error: %s", uv_strerror (rc));
  return G_ERR;
}


static void
api_newconn (uv_stream_t *req, int status)
{
  Api *api = (Api *) req;
  int rc = 0;

  debug ("init client");
  rc = uv_pipe_init ((SERVER_FROM_API (api))->loop, &api->client, 0);
  if (rc < 0)
    goto error;

  debug ("accept client");
  rc = uv_accept ((uv_stream_t *) api, (uv_stream_t *) &api->client);
  if (rc < 0)
    goto error;

  debug ("listen client");
  rc = uv_read_start ((uv_stream_t *) &api->client, api_alloc_cb, api_cb);
  if (rc < 0)
    goto error;

  return;

error:
  debug ("closing client");
  uv_close ((uv_handle_t *) &api->client, NULL);
  if (rc)
    debug ("error: %s", uv_strerror (rc));
  return;
}


static void
api_alloc_cb (uv_handle_t *req, size_t suggested, uv_buf_t *buf)
{
  Api *api = API_FROM_CLIENT (req);
  
  if (api->base_alloc) {
    debug ("buffer double allocated");
    server_fatal (SERVER_FROM_API (api), 1);
  }

  debug ("allocating buffer");
  if (!api->reusable_base)
    api->reusable_base = malloc (suggested);

  // memset (api->reusable_base, 0, suggested);
  buf->base = api->reusable_base;
  buf->len = suggested;

  api->base_alloc = true;
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

  debug ("\"%.*s\"", (int) nread, buf->base);

  uv_buf_t echo = { .base = buf->base, .len = nread };
  api_send (SERVER_FROM_API(API_FROM_CLIENT(client)), &echo, NULL);


done:
  debug ("done");
  // done
  goto finally;

reject:
  debug ("reject");
  // reject
  goto finally;

finally:
  API_FROM_CLIENT(client)->base_alloc = false;
  return;
}
