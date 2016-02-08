#include "test_send.h"


const char *JSON = "{\"id\": \"SERVER_ID\", \
\"event\": \"message\", \
\"body\":  \"hello server\" \
}";

int main ()
{
  return send_message (default_host_ip, default_host_port, JSON, strlen(JSON));
}
