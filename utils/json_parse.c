#include "utils/json_parse.h"


#define TOKSIZE(tok) ((size_t) (tok->end - tok->start))

/* instead of returning a null pointer return this, which means
 * thats the given key does not exist.
 */
static const JsonVal JSONVAL_UNDEFINED = {{0}};


static inline int
comparator_token (void *lhs, size_t lsize, void *rhs, size_t rsize)
{
  return comparator_string_ic (*(char **) lhs, lsize,
                               *(char **) rhs, rsize);
}

static inline uint32_t
hash_token (void *key, size_t key_size)
{
  return hash_djb2_ic (*(char **) key, key_size);
}


JsonParser*
json_parser_new ()
{
  JsonParser *j = malloc (sizeof(JsonParser));
  if (!j)
    goto error;

  j->keymap = map_new (JsonVal, node, key);
  
  if (!j->keymap)
    goto error;
  j->keymap->hash = hash_token;
  j->keymap->cmp = comparator_token;

  jsmn_init (&j->parser);
  j->toklen = JSON_MAX_TOKENS;
  j->tokens = calloc (j->toklen, sizeof (jsmntok_t));
  if (!j->tokens)
    goto error;

  j->vals = calloc (j->toklen / 2, sizeof (JsonVal));
  if (!j->vals)
    goto error;

  j->start = 0;
  j->size = 0;
  return j;

error:
  json_parser_destroy (j);
  return NULL;
}


JsonStatus
json_parser_clear (JsonParser *j)
{
  map_clear (j->keymap);
  jsmn_init (&j->parser);
  memset (j->tokens, 0, j->toklen * sizeof (jsmntok_t));
  memset (j->vals, 0, (j->toklen / 2) * sizeof (JsonVal));
  j->start = 0;
  j->size = 0;
  return JSON_OK;
}


JsonStatus
json_parser_destroy (JsonParser *j)
{
  if (!j)
    return JSON_ERR;
  if (map_free (j->keymap) != MAP_OK)
    return JSON_ERR;
  free (j->tokens);
  free (j->vals);
  free (j);
  return JSON_OK;
}


JsonStatus
json_parse_src (JsonParser *j, char *src, size_t srclen)
{
  if (json_parser_clear(j) != JSON_OK)
    return JSON_ERR;

  int rc;
  rc = jsmn_parse(&j->parser, src, srclen, j->tokens, j->toklen);

  if (rc < 0) goto error;
  if (!j->tokens[0].type) goto error;

  j->start = j->tokens[0].start;
  j->size = j->tokens[0].end - j->start;

  // add all tokens to key map.
  // start at index 1 because the 0th element
  // is just the root json object.
  jsmntok_t *keytok, *valtok;
  JsonVal *p = j->vals, *val;
  for (size_t i = 1; i < j->toklen; i += 2) {
    keytok = &j->tokens[i];
    if (!keytok->type)
      // done adding.
      break;
    if (!(i + 1 < j->toklen))
      // should end up with an even number of KV pairs
      goto error;
    valtok = &j->tokens[i+1];
    
    // only accept key-value pairs with string keys and
    // string, primitive, or array values.
    if (keytok->type != JSMN_STRING ||
        (valtok->type != JSMN_STRING &&
         valtok->type != JSMN_PRIMITIVE &&
         valtok->type != JSMN_ARRAY))
      goto error;

    val = p++;
    val->key = &src[keytok->start];
    val->size = TOKSIZE(valtok);

    // check for duplicate keys
    if (map_get (j->keymap, &val->key, TOKSIZE(keytok)))
      goto error;

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

    if (map_add (j->keymap, val, TOKSIZE(keytok)) != MAP_OK)
      goto error;
  }

  return JSON_OK;

error:
  json_parser_clear (j);
  return JSON_ERR;
}


JsonVal*
json_lookup (JsonParser *j, char *key, size_t key_size)
{
  JsonVal *val;
  return (val = map_get(j->keymap, &key, key_size))
         ? val
         : (JsonVal *) &JSONVAL_UNDEFINED;
}
