#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <new>

// demonstrates a case where hiding happens instead of overriding, and compiler
// would emit a warning about it.

class Base {
public:
  virtual ~Base() = default;
  virtual void f() const { std::cout << "Base::f\n"; }
};

class Derived : public Base {
public:
  // with -Wall clang warns about this hiding behavior
  virtual void f() { std::cout << "Derived::f\n"; }
};

int main() {
  Base* pd = new Derived();
  pd->f(); // hiding! not override!
  delete pd;
  return 0;
}
