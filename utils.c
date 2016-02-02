#include "utils.h"


double
utc_now ()
{
  static struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * (uint64_t) 1000000 + tv.tv_usec) / ((double) 1e6);
}
