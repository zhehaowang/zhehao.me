#include <iostream>
#include <string>
#include <vector>

// demonstrates behavior of dynamic_cast in an inheritance hierarchy, wrong ways
// to use dynamic_cast, and cases where static casting base pointer to derived
// pointer may result in pointer values being different.

class Base1 {
public:
  virtual ~Base1() {}
  virtual void someBase1Call() {}
};

class Base2 {
public:
  virtual ~Base2() {}
  virtual void someBase2Call() {}
};

class Derived1 : public Base1 {
public:
  void someDerived1Call() {}
};

class Derived2 : public Derived1 {
public:
  void someDerived2Call() {}
};

class MultiDerived: public Base1, public Base2 {};

int main() {
  Derived1* pd1 = new Derived1();
  Derived2* pd2 = new Derived2();
  Base1*    pbd1 = pd1;

  // all the wrong way to use dynamic_cast
  if (Derived2* temp = dynamic_cast<Derived2*>(pd1)) {
    std::cout << "pointer to base is casted down\n";
    // fails
  }

  if (Derived1* temp = dynamic_cast<Derived1*>(pd1)) {
    std::cout << "pointer isn't casted\n";
    // passes
  }

  if (Derived1* temp = dynamic_cast<Derived1*>(pbd1)) {
    std::cout << "realized pointer to base can be casted as it actually points"
              << " to derived.\n";
    // passes
  }

  if (Derived2* temp = dynamic_cast<Derived2*>(pbd1)) {
    std::cout << "base pointer further casted down albeit it doesn't point to"
              << " an object of that type\n";
    // fails
  }

  MultiDerived* md = new MultiDerived();
  // reinterpret_cast is implementation dependent, using it makes your code not
  // portable
  Base2 *pbmd = static_cast<Base2*>(md);
  // note how the value of pbmd differs from that of md: assigning pointer to
  // derived to pointer to base causes the two to have different values.

  // this is an effect of multiple inheritance, where the Base2 pointer points
  // to only a part of what MultiDerived pointer points to (8 bytes after).
  std::cout << reinterpret_cast<long>(md) << "\n";
  std::cout << reinterpret_cast<long>(pbmd) << "\n";

  delete pd1;
  delete pd2;
  delete md;
  return 0;
}