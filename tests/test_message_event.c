#include "test_send.h"


const char *JSON = "{\"event\": \"message\", \"body\": \"hello server\"}";

int main ()
{
  return send_message ("127.0.0.1", 9670, JSON, strlen(JSON));
}
