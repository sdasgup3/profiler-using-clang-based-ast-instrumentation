
#include <stdio.h> /* printf */

int fibonacci_rec(int);
int fibonacci_iter(int);
int frequency_of_primes(int);

void runner(int iter) {
  for(int i = 0;i < iter; i++) {
    printf("Primes: %d\n\n", frequency_of_primes(999999));
    printf("Fibonacci (Iter): %d\n\n", fibonacci_iter(20));
    printf("Fibonacci (rec): %d\n\n", fibonacci_rec(20));
  }
}

int main() {
  for(int count = 0;count < 5 ; count++)
    runner(count);
  return 0;
}
