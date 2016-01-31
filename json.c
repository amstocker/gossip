#include "json.h"

  
static uint32_t
hash_val (void *key, size_t _)
{
  (void) _;  // unused
  JsonVal *val = (JsonVal *) key;
  
  return hash_djb2_ic((void *) &val->builder->src[val->tok->start],
                      (size_t) val->tok->size);
}


static int
compare_val (void *lhs, void *rhs, size_t _)
{
  (void) _;  // unused
  JsonVal *lval = (JsonVal *) lhs,
          *rval = (JsonVal *) rhs;
  
  int cmp =  comparator_string_ic((void *) &lval->builder->src[lval->tok->start],
                                  (void *) &rval->builder->src[rval->tok->start],
                                  MIN(lval->tok->size, rval->tok->size));
  
  if (cmp == 0 && lval->tok->size != rval->tok->size)
    return lval->tok->size - rval->tok->size;
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

  b->vals = calloc(b->toklen, sizeof(JsonVal));
  if (!b->vals)
    goto error;
  for (size_t i = 0; i < b->toklen; i++)
    b->vals[i].builder = b;

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
  for (size_t i = 0; i < b->toklen; i++)
    b->vals[i].tok = NULL;
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

  if (rc < 0) {
    json_builder_clear(b);
    return JSON_ERR;
  }

  return JSON_OK;
}
