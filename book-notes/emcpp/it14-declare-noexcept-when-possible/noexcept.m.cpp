#include <iostream>
#include <string>
#include <algorithm>

int main() {
  int x[2][3];
  int y[2][3];

  using std::swap;
  // swap's noexcept is contingent upon the noexcept-ness of the given parameters
  std::cout << noexcept(swap(x, y)) << "\n";

  return 0;
}