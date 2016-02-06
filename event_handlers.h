#include "gossip.h"


typedef Status (*EventHandler) (EventHandle *event);

// X_event_handler
Status message_event_handler (EventHandle *event);
