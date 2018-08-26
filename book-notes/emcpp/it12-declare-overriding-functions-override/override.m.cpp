#include <iostream>
#include <string>

using namespace std;

class Base {
public:
  virtual void func(int i) { cout << "base " << i << "\n"; }

  virtual ~Base() {}
};

class Derived : public Base {
public:
  virtual void func(unsigned int i) /*override*/ { cout << "derived " << i << "\n"; }
  // this is not a valid override. Without 'override' keyword, the compiler won't tell you it's not.

  //virtual void func(const int i) override { cout << "derived " << i << "\n"; }
  // this is a valid override. Looks like cv qualifier difference is Ok? Is there a reason why?
};

int main() {
  Base* d = new Derived();
  int x = 5;
  d->func(x);
  delete d;
  return 0;
}