#include <stdio.h>

#include "deps/include/uv.h"
#include "gossip.h"


Status send_message (const char *ip, short port, const char *msg, size_t len);
