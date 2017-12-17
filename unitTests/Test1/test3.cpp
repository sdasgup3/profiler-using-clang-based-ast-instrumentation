#include <string.h>
#define POVMS_Sys_Memmove(a, b, c) strncpy(a, b, c)

char arr[100];
char* func(int size, int index) {
  if (index < size)
    return POVMS_Sys_Memmove((&(arr[0])), (&(arr[size - index])),
                      sizeof(char) * (size - index));
}

int main() {
  func(66, 56);
  return 0;
}
