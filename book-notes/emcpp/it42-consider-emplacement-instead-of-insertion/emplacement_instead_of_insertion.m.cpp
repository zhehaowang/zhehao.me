#include <iostream>
#include <string>
#include <vector>
#include <regex>

using namespace std::literals;

void deleteMyInt(int* ptr) {
  delete ptr;
}

int main() {
  std::vector<std::string> vecString;
  // less efficient
  vecString.push_back("bad stuff");
  // more efficient
  vecString.emplace_back("good stuff");

  std::string str("neutral stuff");
  // equally efficient
  vecString.push_back(str);
  vecString.emplace_back(str);

  std::vector<std::regex> vecRegexes;
  // does not compile, phew
  //vecRegexes.push_back(nullptr);
  // compiles, leads to UB: crashes this program
  //vecRegexes.emplace_back(nullptr);

  std::vector<std::shared_ptr<int>> vecSharePtrInt;
  // more exception safe
  vecSharePtrInt.push_back({new int(3), deleteMyInt});
  // less exception safe
  vecSharePtrInt.emplace_back(new int(3), deleteMyInt);
  return 0;
}