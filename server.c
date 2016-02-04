#include "server.h"
#include "event.h"


int main () {
  GServer server;
  server.loop = uv_default_loop ();

  event_init (&server);
  event_start (&server);

  uv_run (server.loop, UV_RUN_DEFAULT);

  return 0;
}
