#include "json.h"


#define TOKSIZE(tok) ((size_t) (tok->end - tok->start))

static uint32_t
hash_val (void *key, size_t _)
{
  (void) _;  // unused
  JsonVal *val = (JsonVal *) key;
  
  // hash slice of original json source.
  return hash_djb2_ic(val->keysrc, TOKSIZE(val->keytok));
}


static int
compare_val (void *lhs, void *rhs, size_t _)
{
  (void) _;  // unused
  size_t lsize = TOKSIZE(((JsonVal *) lhs)->keytok),
         rsize = TOKSIZE(((JsonVal *) rhs)->keytok);

  // compare slices of original json
  // source string (ignoring case).
  int cmp =  comparator_string_ic(((JsonVal *) lhs)->keysrc,
                                  ((JsonVal *) rhs)->keysrc,
                                  MIN(lsize, rsize));
  
  // if first N chars are equal but size
  // doesn't match, cannot return 0.
  if (cmp == 0 && lsize != rsize)
    return lsize - rsize;
  else
    return cmp;
}


JsonBuilder*
json_builder_new ()
{
  JsonBuilder *b = malloc(sizeof(JsonBuilder));
  if (!b)
    goto error;
  b->src = NULL;
  b->srclen = 0;

  b->keymap = map_new_with_offsets(offsetof(JsonVal, node), 0, 0);
  if (!b->keymap)
    goto error;
  b->keymap->hash = hash_val;
  b->keymap->cmp = compare_val;

  jsmn_init(&b->parser);
  b->toklen = JSON_MAX_TOKENS;
  b->tokens = calloc(b->toklen, sizeof(jsmntok_t));
  if (!b->tokens)
    goto error;

  b->vals = calloc(b->toklen / 2, sizeof(JsonVal));
  if (!b->vals)
    goto error;

  return b;

error:
  if (b) {
    map_free(b->keymap);
    free(b->tokens);
    free(b->vals);
    free(b);
  }
  return NULL;
}


JsonStatus
json_builder_clear (JsonBuilder *b)
{
  b->src = NULL;
  b->srclen = 0;
  map_clear(b->keymap);
  jsmn_init(&b->parser);
  memset(b->tokens, 0, b->toklen * sizeof(jsmntok_t));
  memset(b->vals, 0, (b->toklen / 2) * sizeof(JsonVal));
  return JSON_OK;
}


JsonStatus
json_builder_destroy (JsonBuilder *b)
{
  if (!b)
    return JSON_ERR;
  if (map_free(b->keymap) != MAP_OK)
    return JSON_ERR;
  free(b->tokens);
  free(b->vals);
  free(b);
  return JSON_OK;
}


JsonStatus
json_parse_src (JsonBuilder *b, char *src, size_t srclen)
{
  if (json_builder_clear(b) != JSON_OK)
    return JSON_ERR;

  b->src = src;
  b->srclen = srclen;

  int rc;
  rc = jsmn_parse(&b->parser, src, srclen, b->tokens, b->toklen);

  if (rc < 0) goto error;

  // add all tokens to key map
  jsmntok_t *keytok, *valtok;
  JsonVal *val;
  for (size_t i = 1; i < b->toklen; i += 2) {
    keytok = &b->tokens[i];
    if (!keytok->type)
      // done adding.
      break;
    if (!(i + 1 < b->toklen))
      // should end up with an even number of KV pairs
      goto error;
    valtok = &b->tokens[i+1];
    
    // only accept key-value pairs with string keys and
    // string or primitive values.
    if (keytok->type != JSMN_STRING ||
        (valtok->type != JSMN_STRING &&
         valtok->type != JSMN_PRIMITIVE))
      goto error;
    
    val = &b->vals[i];
    val->keytok = keytok;
    val->keysrc = &b->src[val->keytok->start];

    // parse type of value
    val->valtok = valtok;
    val->valsrc = &b->src[val->valtok->start];
    switch (*(val->valsrc)) {
      case 't':
        // true
        val->as_bool = 1;
      case 'f':
        // false
        val->as_bool = 0;
        val->type = JSON_BOOL;
        break;
      case '0' ... '9':
      case '-':
        if (sscanf(val->valsrc, "%lf", &val->as_double) < 1)
          goto error;
        val->type = JSON_DOUBLE;
        break;
      case 'n':
        // null
        val->as_null = NULL;
        val->type = JSON_NULL;
        break;
      default:
        val->as_string = val->valsrc;
        val->type = JSON_STRING;
    }
    
    if (map_add(b->keymap, val) != MAP_OK)
      goto error;
  }

  return JSON_OK;

error:
  json_builder_clear(b);
  return JSON_ERR;
}


JsonVal*
json_lookup (JsonBuilder *b, char *key, size_t keylen)
{
  // map is expecting a JsonVal*, so for a clean
  // API we need some hacky dummy vars.
  static jsmntok_t dummy_tok = { JSMN_STRING };
  static JsonVal dummy_val = { NULL, NULL, &dummy_tok };
  
  dummy_val.keysrc = key;
  dummy_tok.end = keylen;
  return map_get(b->keymap, &dummy_val);
}
