#include <iostream>
#include <string>
#include <memory>

// demonstrates the rule of prepending 'typename' before nested dependent type
// names (except when such names are those of base classes in inheritance, or
// appear in a member initialization list)

int x = 10;

class MyClass {
public:
  class Nested {
  public:
    int d_data;
  };

  void doStuff() const { std::cout << "nested do stuff\n"; }

  Nested d_nested;

  // compiler thought: what if you have this? This would become ambiguous
  //static int Nested;

  static int member;
};

template <typename T>
void templateFunc(const T& t) {
  typename T::Nested n; // 'typename' is required here, nested dependent type
                        // name
  T::member *x;         // compiler may think you have this instead: 'member' is
                        // a static data member, and x is something to multiply
                        // it with. So this is a multiplication whose result is
                        // not used, instead of a pointer declaration.
                        // this generates a compiler warning.
  t.doStuff();
}

template <typename T>
class TemplateBase {
public:
  class Nested {

  };
  T data;
};

template <typename T>
class Derived : public TemplateBase<T>::Nested {
public:
  // in the declaration of derived, you don't need a 'typename' to specify
  // TemplateBase<T>::Nested being a nested dependent type name
  Derived() : TemplateBase<T>::Nested() { std::cout << "derived ctor\n"; }
  // similarly with member initialization list
};

int main() {
  MyClass m;
  templateFunc(m);

  Derived<int> d;
  return 0;
}