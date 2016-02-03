#pragma once

#include "uv.h"


/* API Spec
 * --------
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


