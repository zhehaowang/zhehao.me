#include <iostream>
#include <string>
#include <vector>

void f(const std::vector<int>& v) {
  for (const auto& s: v) {
    std::cout << s << "\n";
  }
}

template<typename... Ts>
void fwd(Ts&&... params) {
  f(std::forward<Ts>(params)...);    // forward them to f
}

int main() {
  f({1, 2, 3});
  //fwd({1, 2, 3}); // fails to compile
  auto initializerList = {1, 2, 3};
  fwd(initializerList); // all good
  return 0;
}