#include "gossip.h"


/* Event Handler Map
 * -----------------
 * 
 * The udp callback will match the EventHandler with the value in the "event"
 * field in the json sent to the server.
 *
 */
typedef Status (*EventHandler) (Event *event);

#define EVENT_MAP \
  { "message"  , 7       , message_event_handler    }


// X_event_handler
Status message_event_handler (Event *event);
