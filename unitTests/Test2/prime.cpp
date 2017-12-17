#include <math.h> /* sqrt */

int frequency_of_primes(int n) {
  int i, j;
  int freq = n - 1;
  for (i = 2; i <= n; ++i)
    for (j = sqrt(i); j > 1; --j)
      if (i % j == 0) {
        --freq;
        break;
      }
  return freq;
}
