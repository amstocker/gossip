#include <float.h>
#include <string.h>
#include "utils/json_build.h"


/* Maximum ammount of buffer needed to express a
 * double as a string.
 *  (source: http://stackoverflow.com/questions/1701055/what-is-the-maximum-length-in-chars-needed-to-represent-any-double-value)
 *
 * for non-scientific notation (%lf):
 *  #define DBL_SIZE (3 + DBL_MANT_DIG - DBL_MIN_EXP)
 *
 * for scientfici notation (%e):
 *  #define DBL_SIZE 24
 */
#define DBL_SIZE (3 + DBL_MANT_DIG - DBL_MIN_EXP)
#define DBL_FMT "%lf"


JsonStatus
json_builder_init (JsonBuilder *j, char *buf, size_t buf_size)
{
  j->buf = buf;
  j->buf_size = buf_size;
  j->pos = 0;
  j->nkeys = 0;
  return JSON_OK;
}


JsonStatus
json_builder_start (JsonBuilder *j)
{
  if (j->pos + 1 >= j->buf_size)
    return JSON_ERR;

  j->buf[j->pos] = '{';
  j->pos++;
  return JSON_OK;
}


JsonStatus
json_builder_end (JsonBuilder *j)
{
  if (j->pos + 1 >= j->buf_size)
    return JSON_ERR;

  j->buf[j->pos] = '}';
  j->pos++;
  return JSON_OK;
}


JsonStatus
json_builder_add (JsonBuilder *j, char *key, size_t key_size,
                                  void *val, size_t val_size,
                                  JsonValType val_type)
{
  // maybe one comma (,), two quotes ("), and one colon (:)
  size_t needed = (j->nkeys > 0 ? 1 : 0) + key_size + 3;

  switch (val_type) {
    case JSON_STRING:
      needed += val_size;
      needed += 2;  // two quotes (")
      break;
    case JSON_DOUBLE:
      needed += DBL_SIZE;
      break;
    case JSON_BOOL:
      // 5 chars for "false"
      needed += 5;
      break;
    case JSON_NULL:
      // 4 chars for "null"
      key_size += 4;
      break;
    case JSON_ARRAY:
      // don't support arrays yet
    case JSON_UNDEFINED:
      return JSON_ERR; 
  }
  needed += 1;  // for null-term

  if (j->pos + needed >= j->buf_size)
    return JSON_ERR;

  // save start position in case of error
  size_t start = j->pos;
  
  // copy key
  if (j->nkeys > 0)
    j->buf[j->pos++] = ',';
  j->buf[j->pos++] = '"';
  strncpy (&j->buf[j->pos], key, key_size);
  j->pos += key_size;
  j->buf[j->pos++] = '"';
  j->buf[j->pos++] = ':';
  
  // copy val
  size_t copied;
  switch (val_type) {
    case JSON_STRING:
      j->buf[j->pos++] = '"';
      strncpy (&j->buf[j->pos], val, val_size);
      j->pos += val_size;
      j->buf[j->pos++] = '"';
      break;
    case JSON_DOUBLE:
      copied = sprintf (&j->buf[j->pos], DBL_FMT, *(double *) val);
      if (copied < 0)
        goto error;
      j->pos += copied;
      break;
    case JSON_BOOL:
      copied = sprintf (&j->buf[j->pos], "%s", (*(int *) val) ? "true" : "false");
      if (copied < 0)
        goto error;
      j->pos += copied;
      break;
    case JSON_NULL:
      copied = sprintf (&j->buf[j->pos], "%s", "null");
      if (copied < 0)
        goto error;
      j->pos += copied;
      break;
    default:
      goto error;
  }

  j->nkeys++;
  return JSON_OK;

error:
  j->pos = start;
  return JSON_ERR;
}


JsonStatus
json_builder_add_string (JsonBuilder *j, char *key, size_t key_len,
                                         char *val, size_t val_len)
{
  return json_builder_add (j, key, key_len, val, val_len, JSON_STRING);
}


JsonStatus
json_builder_add_double (JsonBuilder *j, char *key, size_t key_len, double val)
{
  return json_builder_add (j, key, key_len, &val, 0, JSON_DOUBLE);
}


JsonStatus
json_builder_add_bool (JsonBuilder *j, char *key, size_t key_len, int val)
{
  return json_builder_add (j, key, key_len, &val, 0, JSON_BOOL);
}


JsonStatus
json_builder_add_null (JsonBuilder *j, char *key, size_t key_len)
{
  return json_builder_add (j, key, key_len, NULL, 0, JSON_NULL);
}
