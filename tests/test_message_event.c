#include "test_send.h"


const char *JSON = "{\"event\": \"message\", \"body\": \"hello server\"}";

int main ()
{
  return send_message (default_host_ip, default_host_port, JSON, strlen(JSON));
}
