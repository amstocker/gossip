#include <stdio.h>
#include <string.h>
#include "json.h"

static char *JSON_SRC = "{\"Andrew\": 25, \"Rules?\": \"Yes.\"}";

const char *TYPES[5] = {"undefined", 
                        "object",
                        "array",
                        "string",
                        "primitive"};

void dump () {
  size_t len = strlen(JSON_SRC);
  for (size_t i = 0; i < len; i++)
    printf("  %c", JSON_SRC[i]);
  printf("\n");
  for (size_t i = 0; i < len; i++)
    printf("%*lu", 3, i);
  printf("\n");
}


int main() {
  JsonBuilder *jb = json_builder_new();

  json_parse_src(jb, JSON_SRC, strlen(JSON_SRC));

  jsmntok_t *tok = jb->tokens;

  dump();
  while (tok->type) {
    printf("type: %s, start: %i, end: %i, size: %i\n",
           TYPES[tok->type], tok->start, tok->end, tok->size);
    tok++;
  }

  
  JsonVal *val;

  val = json_lookup(jb, "andrew", 6);
  printf("get 'andrew': %lf\n", val->as_double);
  
  val = json_lookup(jb, "rules?", 6);
  printf("get 'rules?': %.*s\n", JSON_VAL_TOKLEN(val), val->as_string);
  
  return 0;
}
