#include "utils/time.h"


Time
time_now ()
{
  static struct timeval tv;

  if (gettimeofday(&tv, NULL))
    return 0;
  return tv.tv_sec * (Time) 1000
         + tv.tv_usec / (Time) 1000;
}
