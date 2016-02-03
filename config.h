#include <stdio.h>
#include <stdlib.h>

#include "json.h"


typedef struct {
  char *filename;
  char *contents;
  JsonBuilder *json;
} ServerConfig;


ServerConfig *config_from_file (char *filename);
ServerConfig *config_from_json (FILE *fp);
void config_destroy (ServerConfig *conf);
