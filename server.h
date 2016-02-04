#pragma once

#include <stdlib.h>
#include "uv.h"

#include "json.h"
#include "map.h"
#include "utils.h"
#include "event.h"


/* Configuration
 * -------------
 *
 * Config file is a json file, for example "gossip.conf.json".
 *
 * User must provide at least:
 *  - username
 *  - array of peers in the form "ip4:port" or "[ip6]:port"
 *  - logfile?
 *
 */
static const char *default_host_ip = "0.0.0.0";
static const short default_host_port = 9670;

static const char *default_unix = "/tmp/gossip.sock";
static const int default_backlog = 128;

static const int default_retries = 3;
static const int default_retry_wait = 10000 /* ms */;

static const int default_ratelim_window = 30000 /* ms */;
static const int default_ratelim_burst_window = 1000 /* ms */;
static const float default_ratelim = 500.0;
static const float default_ratelim_burst = 50.0;

typedef struct GServer GServer;


/* EventHandle
 * -----------
 *
 *  Subclass of uv_udp_t that represents a single event at any one
 *  point in time.  For now there exists just one instance as part of
 *  the server.
 *
 */
typedef struct {
  uv_udp_t req;
  JsonBuilder *json;
  GServer *server;
} EventHandle;


struct GServer {

  uv_loop_t *loop;

  // config
  struct sockaddr *host;
  char *username;
  int retries;
  int retry_wait;
  int ratelim_window;
  int ratelim_window_burst;
  float ratelim;
  float ratelim_burst;

  // handles
  EventHandle event_handle;
  //ApiHandle api_handle;
  uv_pipe_t apisock;

  Map *peers;

};
