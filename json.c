#include "json.h"


#define TOKSIZE(tok) ((size_t) (tok->end - tok->start))

/* instead of returning a null pointer return this, which means
 * thats the given key does not exist.
 */
static const JsonVal JSONVAL_UNDEFINED = {{0}};


/**
 * Custom hash and comparator functions for plugging into
 * the key map.
 **/

static uint32_t
hash_val (void *key, size_t _)
{
  (void) _;  // unused
  JsonVal *val = (JsonVal *) key;
  // hash slice of original json source.
  return hash_djb2_ic(val->key, val->key_size);
}

static int
compare_val (void *lhs, void *rhs, size_t _)
{
  (void) _;  // unused
  JsonVal *lval = (JsonVal *) lhs,
          *rval = (JsonVal *) rhs;
  // compare slices of original json
  // source string (ignoring case).
  int cmp =  comparator_string_ic(lval->key,
                                  rval->key,
                                  MIN(lval->key_size,
                                      rval->key_size)
                                  );
  // if first N chars are equal but size
  // doesn't match, cannot return 0.
  if (cmp == 0 && lval->key_size != rval->key_size)
    return lval->key_size - rval->key_size;
  else
    return cmp;
}



JsonBuilder*
json_builder_new ()
{
  JsonBuilder *b = malloc(sizeof(JsonBuilder));
  if (!b)
    goto error;

  b->keymap = map_new_with_offsets(offsetof(JsonVal, node),
                                   0,  // use struct itself as the key (0 bytes offset)
                                   0); // size is unused so just pass 0
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

  int rc;
  rc = jsmn_parse(&b->parser, src, srclen, b->tokens, b->toklen);

  if (rc < 0) goto error;

  // add all tokens to key map.
  // start at index 1 because the 0th element
  // is just the root json object.
  jsmntok_t *keytok, *valtok;
  JsonVal *p = b->vals, *val;
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
    
    val = p++;
    val->key = &src[keytok->start];
    val->key_size = TOKSIZE(keytok);
    val->size = TOKSIZE(valtok);

    // parse type of value.
    char *start = &src[valtok->start];
    switch (valtok->type) {
      case JSMN_STRING:
        val->as_string = start;
        val->type = JSON_STRING;
        break;
      case JSMN_PRIMITIVE:
        // parse primitive from first char
        switch (*start) {
          case '0' ... '9':
          case '-':
            // double
            if (sscanf(start, "%lf", &val->as_double) < 1)
              goto error;
            val->type = JSON_DOUBLE;
            break;
          case 't':
            // 'true'
            val->as_bool = 1;
          case 'f':
            // 'false'
            val->as_bool = 0;
            val->type = JSON_BOOL;
            break;
          case 'n':
            // 'null'
            val->as_null = NULL;
            val->type = JSON_NULL;
            break;
          default:
            goto error;
        }
        break;
      default:
        goto error;
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
json_lookup (JsonBuilder *b, char *key, size_t key_size)
{
  // use dummy var for cleaner API.
  static JsonVal dummy_val;
  JsonVal *val;

  dummy_val.key = key;
  dummy_val.key_size = key_size;
  return (val = map_get(b->keymap, &dummy_val))
         ? val
         : (JsonVal *) &JSONVAL_UNDEFINED;
}
