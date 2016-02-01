#include <stdio.h>
#include <string.h>
#include "json.h"

static char *JSON_SRC = "{\"Andrew\": 25, \"Rules?\": \"Yes.\"}";

int main() {
  JsonBuilder *jb = json_builder_new();

  json_parse_src(jb, JSON_SRC, strlen(JSON_SRC));

  JsonVal *val;

  val = json_lookup(jb, "andrew", 6);
  printf("get 'andrew': %lf\n", val->as_double);

  val = json_lookup(jb, "rules?", 6);
  printf("get 'rules?': %.*s\n", (int) val->size, val->as_string);

  json_builder_destroy(jb);  
  return 0;
}
