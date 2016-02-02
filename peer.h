#include <socket.h>


struct Peer {
  char name[64],
  struct sockaddr *addr;  // ipv4 or ipv6
  uint64_t last_recv;     // timestamp in millis

};
