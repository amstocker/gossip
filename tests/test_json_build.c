#include <stdio.h>

#include "gossip.h"
#include "utils/json_build.h"


int main () {
  char buf[4096] = { '\0' };

  JsonBuilder j;

  json_builder_init (&j, buf, 4096);

  json_builder_start (&j);

  json_builder_add_string (&j, "andrew", 6, "rules", 5);
  json_builder_add_double (&j, "he rules this hard", 18, 25);

  json_builder_end (&j);

  debug ("build result: %s\n", buf);

  return 0;
}
