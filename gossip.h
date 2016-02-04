#pragma once

#include <stdlib.h>
#include "deps/include/uv.h"
#include <uuid/uuid.h>

#include "utils/json.h"
#include "utils/map.h"
#include "utils/macros.h"



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



typedef enum {
  G_OK,
  G_ERR
} GStatus;

typedef struct GServer GServer;



/* Peer
 * ----
 *
 *
 */

#define PEER_MAX_NAME_LEN 63

typedef struct {
  uuid_t id;
  char name[PEER_MAX_NAME_LEN + 1];
  struct sockaddr *addr;  // ipv4 or ipv6

  uuid_t msg_last_id;
  uint64_t msg_last_time;     // timestamp in millis
  uint32_t msg_last_hash;

  // rate limiting
  float msg_rate;
  float msg_rate_burst;
} Peer;

Peer *peer_new ();
GStatus peer_set_name (Peer *p, const char *buf, size_t len);
GStatus peer_set_addr (Peer *p, const struct sockaddr *addr);



/* EventHandle
 * -----------
 *
 *  Subclass of uv_udp_t that represents a single event at any one
 *  point in time.  For now there exists just one instance as part of
 *  the server.
 *
 */

#define SERVER_FROM_EVENT(P) (container_of(P, GServer, event_handle))

typedef struct {
  uv_udp_t req;
  JsonBuilder *json;
} EventHandle;

GStatus event_init (GServer *server);



/* Api
 * ---
 *
 * Send commands varint-length-prefixed like so:
 *
 *  | varint | 1 b |    N bytes    |
 *  |<prefix>|<cmd>|<message-bytes>|
 *
 * Commands:
 *    
 *  M <body>  := new message
 *  G <n>     := get new messages
 *  Q         := quit
 *
 */

#define SERVER_FROM_API(P) (container_of(P, GServer, api_handle))

typedef struct {
  uv_pipe_t req;
  // ...
} ApiHandle;

GStatus api_init (GServer *server);



/* Server
 * ------
 *
 *
 */
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

  Map *peers;
  
  // handles
  EventHandle event_handle;
  ApiHandle api_handle;
};
