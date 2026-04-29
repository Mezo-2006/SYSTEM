#include <iostream>
using namespace std;

int main() {
  int x = 5, y = 3;
  int z = (x + y) * (x - y);
  switch (z) {
    case 16: z = (2 + 3) * (4 - 1); break;
    default: z = 0; break;
  }
  return 0;
}
