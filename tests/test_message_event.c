#include "test_send.h"


const char *JSON = "{\"event\": \"message\", \"body\": \"hello server\"}";

int main ()
{
  return send_message (JSON, strlen(JSON));
}
