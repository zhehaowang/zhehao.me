#include <iostream>
#include <string>

// demonstrates when not specified, the copycon (assignment opr) of a derived class does not
// by default invoke the copycon (assignment opr) of the base

class Parent {
public:
  Parent() : d_x(10) { std::cout << "Parent default ctor\n"; }
  Parent(const Parent& rhs) : d_x(rhs.d_x) { std::cout << "Parent copycon\n"; }
  Parent& operator=(const Parent& rhs) {
    d_x = rhs.d_x;
    std::cout << "Parent assignment opr\n";
    return *this;
  }
  int x() const { return d_x; }
  void setX(int x) { d_x = x; }
private:
  int d_x;
};

class Child : public Parent {
public:
  Child() : d_y(5) { std::cout << "Child default ctor\n"; }
  Child(const Child& rhs) : Parent(rhs), d_y(rhs.d_y) { std::cout << "Child copy ctor\n"; }
  Child& operator=(const Child& rhs) {
    std::cout << "Child assignment opr\n";
    
    // remember to call Parent's assignment operator!
    Parent::operator=(rhs);
    
    d_y = rhs.d_y;
    return *this;
  }
private:
  void init() {} // to avoid duplication between assignment opr and copycon,
                 // one could put shared code in a private init()

  int d_y;
};

int main() {
  Child c;
  c.setX(20);
  Child d(c);
  d = c;
  std::cout << d.x() << "\n";
  return 0;
}