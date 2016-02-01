#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "utils.h"
#include "thirdparty/jsmn.h"
#include "collections/hash.h"
#include "collections/comparator.h"
#include "collections/map.h"


#define JSON_MAX_TOKENS 256

typedef enum {
  JSON_OK,
  JSON_ERR
} JsonStatus;

typedef enum {
  JSON_UNDEFINED=0,
  JSON_STRING,
  JSON_DOUBLE,
  JSON_BOOL,
  JSON_NULL
} JsonValType;

typedef struct JsonBuilder JsonBuilder;
typedef struct JsonVal JsonVal;

struct JsonVal {
  union {
    char *as_string;
    int as_bool;
    double as_double;
    void *as_null;
  };
  JsonValType type;
  size_t size;

  char *key;
  size_t key_size;
  MapNode node;
};

/**
 * Reusable map and token array for quick parsing of json with ability to
 * query for keys.  Each builder can only focus on one json source string at
 * a time.
 **/
struct JsonBuilder {
  JsonVal *vals;
  Map *keymap;
  
  // for use by the jsmn parsing library
  jsmn_parser parser;
  jsmntok_t *tokens;
  size_t toklen;
};


JsonBuilder *json_builder_new ();
JsonStatus json_builder_clear (JsonBuilder *b);
JsonStatus json_builder_destroy (JsonBuilder *b);
JsonStatus json_parse_src (JsonBuilder *b, char *src, size_t srclen);
JsonVal *json_lookup (JsonBuilder *b, char *key, size_t keylen);
