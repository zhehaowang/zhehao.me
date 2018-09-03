#include <iostream>
#include <string>
#include <array>
#include <vector>

int main() {
  // note the difference in behaviors between move on array and vector
  
  std::array<int, 5> a = {1, 2, 3, 4, 5};
  // different from containers having pointer on stack pointing to content stored on heap,
  // std::array encapsulates fixed size array and place its content on stack.
  // its move is then of the same complexity as its copy.
  auto b(std::move(a));
  std::cout << "a (array), after move: \n";
  for (const auto& s: a) {
    std::cout << s << ' ';
  }
  std::cout << "\nb (array), after move: \n";
  for (const auto& s: b) {
    std::cout << s << ' ';
  }

  std::vector<int> a1 = {1, 2, 3, 4, 5};
  auto b1(std::move(a1));
  std::cout << "\na1 (vector), after move: \n";
  for (const auto& s: a1) {
    std::cout << s << ' ';
  }
  std::cout << "\nb1 (vector), after move: \n";
  for (const auto& s: b1) {
    std::cout << s << ' ';
  }

  std::cout << "\n";
  int&& x = 5;
  auto&& y(std::move(x));
  y += 1;
  std::cout << x << "\n";
  return 0;
}