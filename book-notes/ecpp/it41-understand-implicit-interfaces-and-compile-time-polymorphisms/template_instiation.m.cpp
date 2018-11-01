#include <iostream>
#include <string>
#include <memory>

// demonstrates implicit interface and compile time polymorphism in template
// instantiation

template<typename T>
void largerThanTen(T& w) {
  if (w.size() > 10) {
    std::cout << "larger than 10\n";
  } else {
    std::cout << "no larger than 10\n";
  }
}

class C3 {};

class C4 {};

class C2 {
public:
  C2() = default;
  C2(const C3&) {}
  // implicit conversion from C3 to C2

  C4 size() const { return C4(); }
};

class C5 {
public:
  C5() = default;
  C5(const C4&) {}
  // implicit conversion from C4 to C5
};

bool operator>(const C5& lhs, double rhs) {
  return true;
}

int main() {
  C2 obj;
  // using C3 here won't work, as compiler expects the implicit interface to
  // directly contain a call 'size()', rather than anything that this type can
  // implicitly convert to contains a call 'size()'
  largerThanTen(obj);
  return 0;
}