
int fibonacci_rec(int n) {

  if (n <= 1) {
    return n;
  } else {
    return fibonacci_rec(n - 1) + fibonacci_rec(n - 2);
  }
}

int fibonacci_iter(int n) {
  int previous = 1;
  int current = 1;
  int next = 1;
  for (int i = 3; i <= n; ++i) {
    next = current + previous;
    previous = current;
    current = next;
  }

  return next;
}
