#include "utils/json_parse.h"


typedef struct JsonBuilder JsonBuilder;

struct JsonBuilder {
  char  *buf;
  size_t buf_size;
  size_t pos;
  size_t nkeys;
};


JsonStatus json_builder_init (JsonBuilder *j, char *buf, size_t buf_size);
JsonStatus json_builder_start (JsonBuilder *j);
JsonStatus json_builder_end (JsonBuilder *j);
JsonStatus json_builder_add (JsonBuilder *j, char *key, size_t key_size,
                                             void *val, size_t val_size,
                                             JsonValType val_type);
JsonStatus json_builder_add_string (JsonBuilder *j, char *key, size_t key_len,
                                                    char *val, size_t val_len);
JsonStatus json_builder_add_double (JsonBuilder *j, char *key, size_t key_len,
                                                    double val);
JsonStatus json_builder_add_bool (JsonBuilder *j, char *key, size_t key_len,
                                                  int val);
JsonStatus json_builder_add_null (JsonBuilder *j, char *key, size_t key_len);
