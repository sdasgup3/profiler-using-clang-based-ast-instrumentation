// The following header need to be found out for correct instrumentation
// Else the if will not be recognized as a valid if statement becuase of
// parsing error.
#include <climits>

int main() {
  int RedU = 0;
  if (RedU > 0)
    return  UCHAR_MAX;

  return RedU;
}
