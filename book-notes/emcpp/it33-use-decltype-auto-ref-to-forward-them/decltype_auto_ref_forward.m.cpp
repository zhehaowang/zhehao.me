#include <iostream>
#include <string>

void forwarded_func(int& a, int& b, int& c) {
  std::cout << "lvalue variation of func: "
            << a << " "
            << b << " "
            << c << "\n";
}

void forwarded_func(int&& a, int&& b, int&& c) {
  std::cout << "rvalue variation of func: "
            << a << " "
            << b << " "
            << c << "\n";
}

int main() {
// Test of C++14 generic variadic lambdas with perfect forwarding, 
  auto f = [](auto&&... xs) {
    forwarded_func(std::forward<decltype(xs)>(xs)...);
  };
  f(1, 2, 3);
  int a = 4;
  int b = 5;
  int c = 6;
  f(a, b, c);
  return 0;
}