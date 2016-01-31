#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "utils.h"
#include "hash.h"
#include "comparator.h"
#include "map.h"
#include "thirdparty/jsmn.h"


#define JSON_MAX_TOKENS 256

typedef enum {
  JSON_OK,
  JSON_ERR
} JsonStatus;

typedef enum {
  JSON_STRING,
  JSON_BOOL,
  JSON_DOUBLE,
  JSON_NULL
} JsonValType;

typedef struct JsonBuilder JsonBuilder;

typedef struct {
  JsonBuilder *builder;
  jsmntok_t *keytok,
            *valtok;
  MapNode node;

  JsonValType type;
  union {
    char *as_string;
    int as_bool;
    double as_double;
    void *as_null;
  };
} JsonVal;

/**
 * Reusable map and token array for quick parsing of json with ability to
 * query for keys.  Each builder can only focus on one json source string at
 * a time.
 **/
struct JsonBuilder {
  char *src;
  size_t srclen;
  
  Map *keymap;
  
  jsmn_parser parser;
  jsmntok_t *tokens;
  size_t toklen;

  JsonVal *vals;  // vals[0] is root json object
};


#define JSON_VAL_TOKLEN(v) (v->valtok->end - v->valtok->start)

JsonBuilder *json_builder_new ();
JsonStatus json_builder_clear (JsonBuilder *b);
JsonStatus json_parse_src (JsonBuilder *b, char *src, size_t srclen);
JsonVal *json_lookup (JsonBuilder *b, char *key, size_t keylen);
