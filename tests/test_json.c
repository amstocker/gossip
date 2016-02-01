#include <stdio.h>
#include <string.h>

#include "json.h"

static char *JSON_SRC = "{\"Andrew\": 25, \"Rules?\": \"Yes.\"}";


void print_val_for (JsonBuilder *j, char *key)
{
  JsonVal *val = json_lookup(j, key, strlen(key));
  switch (val->type) {
    case JSON_STRING:
      printf("%s: %.*s\n", key, (int) val->size, val->as_string);
      break;
    case JSON_DOUBLE:
      printf("%s: %lf\n", key, val->as_double);
      break;
    case JSON_BOOL:
      printf("%s: %s\n", key, val->as_bool ? "true" : "false");
      break;
    case JSON_NULL:
      printf("%s: (null)\n", key);
      break;
  }
}


int main() {

  JsonBuilder *j = json_builder_new();
  
  json_parse_src(j, JSON_SRC, strlen(JSON_SRC));

  print_val_for(j, "andrew");
  print_val_for(j, "rules?");
  print_val_for(j, "doesnotexist");

  json_builder_destroy(j);  
  return 0;
}
