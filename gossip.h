#pragma once

#include <stdlib.h>
#include "deps/include/uv.h"
#include "deps/include/uuid/uuid.h"

#include "utils/time.h"
#include "utils/json.h"
#include "utils/map.h"
#include "utils/macros.h"


// DEBUG
#include <stdio.h>


typedef enum {
  G_OK  =  0,
  G_ERR = -1,
  G_WARN
} Status;

typedef struct Server Server;
typedef struct Peer Peer;
typedef struct Event Event;
typedef struct Response Response;
typedef struct Api Api;

#define PEER_ID_SIZE 37
#define PEER_MAX_NAME_LEN 63

// id is an encoded uuid
typedef char ID[PEER_ID_SIZE];



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

static const char *default_host_ip = "127.0.0.1";
static const short default_host_port = 9670;

static const char *default_unix = "/tmp/gossip.sock";
static const int default_backlog = 128;

static const int default_retries = 3;
static const int default_retry_wait = 3000 /* ms */;

static const int default_ratelim_window = 30000 /* ms */;
static const int default_ratelim_burst_window = 1000 /* ms */;
static const float default_ratelim = 500.0;
static const float default_ratelim_burst = 50.0;



/* Buffer (buffer.c)
 * -----------------
 *
 * Handle buffer allocations.
 *
 */

void buffer_allocate (uv_handle_t *handle, size_t suggested, uv_buf_t *buf);



/* Peer (peer.c)
 * -------------
 *
 */

struct Peer {
  
  ID id;
  
  // ipv4 or ipv6
  struct sockaddr addr;

  // username
  char name[PEER_MAX_NAME_LEN + 1];

  // last message info
  ID msg_last_id;
  uint64_t msg_last_time;  // timestamp in millis
  uint32_t msg_last_hash;

  // rate limiting metrics
  float msg_rate;
  float msg_rate_burst;

  MapNode node;
};

Status peer_init (Server *server);
Status peer_update (Event *event);



/* Event (event.c)
 * ---------------
 *
 * Subclass of uv_udp_t that represents a single event at any one
 * point in time.  For now there exists just one instance as part of
 * the server.
 *
 */

#define SERVER_FROM_EVENT(P) (container_of (P, Server, event))

struct Event {
  uv_udp_t req;
  JsonBuilder *json;
};

Status event_init (Server *server);
Status event_start (Server *server);



/* Api (api.c)
 * -----------
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

#define SERVER_FROM_API(P) (container_of (P, Server, api))

struct Api {
  uv_pipe_t req;
  // ...
};

Status api_init (Server *server);



/* Response (response.c)
 * ---------------------
 *
 */

struct Response {
  uv_udp_t req;
  // uv_timer_t timer; ???
  uv_buf_t buf;
  int retry;
};



/* Server (server.c)
 * -----------------
 *
 */

struct Server {
  
  uv_loop_t *loop;

  // config
  ID     host_id;
  char  *host_ip;
  short  host_port;
  char  *host_username;
  
  int retries;
  int retry_wait;
  int ratelim_window;
  int ratelim_burst_window;
  float ratelim;
  float ratelim_burst;

  // hash table by id;
  Map *peers;
  
  // handles
  Event event;
  Api api;
};

Status server_init (Server *server);
Status server_run (Server *server);
