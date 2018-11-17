#include <iostream>
#include <string>
#include <vector>
#include <iterator>

// demonstrates vector.insert when given const_iterator, and a usage of
// non-member begin

int main() {
  std::vector<int> v;
  std::vector<int>::const_iterator ci = v.cbegin();
  // on OSX this compiles even with std=gnu++98
  v.insert(ci, 3);

  auto ci2 = std::cbegin(v);
  v.insert(ci2, 4);

  for (auto ci: v) {
  	std::cout << ci << "\n";
  }
  return 0;
}