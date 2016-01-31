#include "json.h"


#define TOKSIZE(tok) ((size_t) (tok->end - tok->start))

static uint32_t
hash_val (void *key, size_t _)
{
  (void) _;  // unused
  JsonVal *val = (JsonVal *) key;
  
  return hash_djb2_ic((void *) &val->builder->src[val->keytok->start],
                      TOKSIZE(val->keytok));
}


static int
compare_val (void *lhs, void *rhs, size_t _)
{
  (void) _;  // unused
  JsonVal *lval = (JsonVal *) lhs,
          *rval = (JsonVal *) rhs;
  size_t lsize = TOKSIZE(lval->keytok),
         rsize = TOKSIZE(rval->keytok);
  int cmp =  comparator_string_ic((void *) &lval->builder->src[lval->keytok->start],
                                  (void *) &rval->builder->src[rval->keytok->start],
                                  MIN(lsize, rsize));
  
  if (cmp == 0 && lsize != rsize)
    return lsize - rsize;
  else
    return cmp;
}


static void
clear_val (JsonVal *val)
{
  val->keytok = NULL;
  val->valtok = NULL;
  val->type = JSON_NULL;
  val->as_null = NULL;
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
  for (size_t i = 0; i < b->toklen / 2; i++) {
    b->vals[i].builder = b;
    clear_val(&b->vals[i]);
  }

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
    clear_val(&b->vals[i]);
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
    if (keytok->type && !(i + 1 < b->toklen))
      goto error;
    if (!keytok->type)
      // done adding.
      break;
    valtok = &b->tokens[i+1];
    
    // only accept key-value pairs with string keys and
    // string or primitive values.
    if (keytok->type != JSMN_STRING ||
        (valtok->type != JSMN_STRING &&
         valtok->type != JSMN_PRIMITIVE))
      goto error;
    
    val = &b->vals[i];
    val->keytok = keytok;
    val->valtok = valtok;

    // parse type
    char *start = &b->src[val->valtok->start];
    switch (*start) {
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
        sscanf(start, "%lf", &val->as_double);
        val->type = JSON_DOUBLE;
        break;
      case 'n':
        // null
        val->as_null = NULL;
        val->type = JSON_NULL;
        break;
      default:
        val->as_string = start;
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
  static jsmntok_t dummy_tok = { JSMN_STRING };
  static JsonBuilder dummy_builder = { NULL };
  static const JsonVal dummy_val = { &dummy_builder, &dummy_tok };
  
  dummy_builder.src = key;
  dummy_tok.end = keylen;
  return map_get(b->keymap, (void*) &dummy_val);
}
