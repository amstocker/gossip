#include "gossip.h"


typedef Status (*EventHandler) (EventHandle *event);

// event_X_handler
Status event_message_handler (EventHandle *event);
