#include <stdio.h>
#include <string.h>
#include "json.h"

static char *JSON_SRC = "{\"Andrew\": 25, \"Rules?\": \"Yes.\"}";


int main() {
  JsonBuilder *jb = json_builder_new();

  json_parse_src(jb, JSON_SRC, strlen(JSON_SRC));



  return 0;
}
