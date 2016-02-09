#include <stdio.h>
#include <string.h>

#include "gossip.h"
#include "utils/json_parse.h"


static char *JSON_SRC = "{\"Andrew\": 25, \"Rules?\": \"Yes.\"}";

JsonParser *j;


void print_val_for (char *key)
{
  JsonVal *val = json_lookup (j, key, strlen(key));
  switch (val->type) {
    case JSON_ARRAY:
    case JSON_STRING:
      debug ("%s: %.*s", key, (int) val->size, val->as_string);
      break;
    case JSON_DOUBLE:
      debug ("%s: %lf", key, val->as_double);
      break;
    case JSON_BOOL:
      debug ("%s: %s", key, val->as_bool ? "true" : "false");
      break;
    case JSON_NULL:
      debug ("%s: null", key);
      break;
    case JSON_UNDEFINED:
      debug ("%s: (undefined)", key);
      break;
  }
}


int main() {
  j = json_parser_new();

  json_parse_src (j, JSON_SRC, strlen (JSON_SRC));

  print_val_for ("andrew");
  print_val_for ("rules?");
  print_val_for ("doesnotexist");

  json_parser_destroy (j);  
  return 0;
}
