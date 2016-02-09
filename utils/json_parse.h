#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include "thirdparty/jsmn.h"
#include "utils/hash.h"
#include "utils/comparator.h"
#include "utils/map.h"


#define JSON_MAX_TOKENS 256

typedef enum {
  JSON_OK   =  0,
  JSON_PART =  1,
  JSON_ERR  = -1
} JsonStatus;

typedef enum {
  JSON_UNDEFINED=0,
  JSON_STRING,
  JSON_DOUBLE,
  JSON_BOOL,
  JSON_ARRAY,
  JSON_NULL
} JsonValType;

typedef struct JsonParser JsonParser;
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
  MapNode node;
};

/**
 * Reusable map and token array for quick parsing of json with ability to
 * query for keys.  Each builder can only focus on one json source string at
 * a time.
 **/
struct JsonParser {
  JsonVal *vals;
  Map *keymap;
  size_t start;
  size_t size;
  
  // for use by the jsmn parsing library
  jsmn_parser parser;
  jsmntok_t *tokens;
  size_t toklen;
};


JsonParser *json_parser_new ();
JsonStatus json_parser_clear (JsonParser *b);
JsonStatus json_parser_destroy (JsonParser *b);
JsonStatus json_parse_src (JsonParser *b, char *src, size_t srclen);
JsonVal *json_lookup (JsonParser *b, char *key, size_t key_size);
