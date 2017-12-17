// These cases will get ignored
int main() {

  bool index = false;
  if (index)
    if (index) {
      index = false;
    }

  if (index)
      index = false;

  return 0;
}
