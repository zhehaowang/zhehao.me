#include <iostream>
#include <string>

// demonstrates overriding and overloading at the same time. Instead of the
// default hiding behavior, say, inheriting all overloads, or some overloads is
// desirable.

// arguably overloading and overriding at the same time is not a good idea

class Base {
public:
  Base() : d_data("base data") {}

  void f() { std::cout << "f (void overload, base)\n"; }
  void f(int x) { std::cout << "f (int overload, base): " << x << "\n"; }
  void g(double d) { std::cout << "g (double overload, base): " << d << "\n"; }
private:
  std::string d_data;
};

class Derived : public Base {
public:
  using Base::f; // such that both overloads of f are visible in this scope and
                 // considered as overload candidates with the same preference
                 // as the f defined here
  Derived() : d_data("derived data") {}

  void f(const std::string& data) {
    std::cout << "f (string overload, derived): " << data << "\n";
  }

  void g(const std::string& data) {
    std::cout << "g (string overload, derived): " << data << "\n";
  }
private:
  std::string d_data;
};

class ImplementationInheritance : private Base {
public:
  // now say if you only want the void version of Base::f() and a string
  // overload defined in this class. (Note that this request does not make sense
  // for public inheritance, as with public inheritance, every function in base
  // class should be applicable to the derived)
  void f(const std::string& data) {
    std::cout << "f (string overload, private inherited): " << data << "\n";
  }

  void f() {
    Base::f();  // simple forwarding function, as 'using' would import all
                // overloads of f()
  }
};

// snippet from the book, whose idea is shown above ImplementationInheritance
// but the particular impl is calling a pure virtual overload.
/*
class Base1 {
public:
  virtual ~Base1() = default;
  virtual void mf1() = 0;
  virtual void mf1(int) {};
};

class Derived1: private Base1 {
public:
  virtual void mf1()                   // forwarding function; implicitly
  { Base1::mf1(); }                    // inline (see Item 30)
};

void func() {
  Derived1 d;
  int x;

  d.mf1();                               // fine, calls Derived::mf1, well
                                         // linker error actually
  //d.mf1(x);                            // error! Base::mf1() is hidden
}
*/

int main() {
  //func();

  std::string data("good");
  int x = 3;
  double y = 4.0;

  Derived d;
  d.f(data); // works
  d.f(x);    // works due to the present of 'using' declaration
  d.f();     // ditto

  //d.g(y);    // compile error

  ImplementationInheritance ii;
  ii.f(data); // works
  ii.f();     // works

  //ii.f(x);    // compile error

  return 0;
}