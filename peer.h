#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <uv.h>
#include <uuid/uuid.h>


#define PEER_MAX_NAME_LEN 63


typedef enum {
  PEER_OK,
  PEER_ERR
} PeerStatus;

typedef struct {
  uuid_t id;
  char name[PEER_MAX_NAME_LEN + 1];
  struct sockaddr *addr;  // ipv4 or ipv6

  uuid_t msg_last_id;
  uint64_t msg_last_time;     // timestamp in millis
  uint32_t msg_last_hash;

  // rate limiting
  float msg_rate;        // 30 second window
  float msg_rate_burst;  // 1 second window
} Peer;


Peer *peer_new_from_addr (struct sockaddr *addr);
