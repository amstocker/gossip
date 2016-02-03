#include "config.h"


ServerConfig*
config_from_file (char *filename)
{
  FILE *fp = fopen (filename, "r");
  if (!fp)
    return NULL;

  ServerConfig *conf = config_from_json (fp);
  if (!conf)
    return NULL;
  fclose(fp);

  conf->filename = filename;
  return conf;
}


ServerConfig*
config_from_json (FILE *fp)
{
  ServerConfig *conf = malloc (sizeof(ServerConfig));
  if (!conf)
    goto error;
  conf->filename = NULL;
  
  // file size
  long fsize;
  if (fseek(fp, 0, SEEK_END))
    goto error;
  fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  
  conf->contents = malloc (fsize);
  if (!conf->contents)
    goto error;
  fread(conf->contents, fsize, 1, fp);
  fseek(fp, 0, SEEK_SET);

  conf->json = json_builder_new ();
  if (!conf->json)
    goto error;

  if (json_parse_src (conf->json, conf->contents, fsize) != JSON_OK)
    goto error;

  return conf;

error:
  config_destroy (conf);
  return NULL;
}


void
config_destroy (ServerConfig *conf)
{
  if (conf) {
    if (conf->contents)
      free (conf->contents);
    if (conf->json)
      json_builder_destroy (conf->json);
    free (conf);
  }
}
