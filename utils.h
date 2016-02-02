#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>


// Macros

#define MIN(x, y) (y ^ ((x ^ y) & -(x < y)))
#define MAX(x, y) (x ^ ((x ^ y) & -(x < y)))


double utc_now ();
