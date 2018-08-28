#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main() {
  std::vector<int> v;
  std::vector<int>::const_iterator ci = v.cbegin();
  // on OSX this compiles even with std=gnu++98
  v.insert(ci, 3);
  return 0;
}