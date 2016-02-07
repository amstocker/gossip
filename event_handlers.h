#include "gossip.h"


typedef Status (*EventHandler) (Event *event);

// X_event_handler
Status message_event_handler (Event *event);
