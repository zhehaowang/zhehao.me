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
  virtual void func(unsigned int i) /*override*/ {
    cout << "derived " << i << "\n";
  }
  // this is not a valid override. Without 'override' keyword, the compiler 
  // won't tell you it's not without -Wall
  // with -Wall it will warn about hiding
  // with 'override' this will be an error

  //virtual void func(const int i) override { cout << "derived " << i << "\n"; }
  // this is a valid override. Looks like cv qualifier difference on parameter
  // is Ok? Is there a reason why?
};

int main() {
  Base* d = new Derived();

  unsigned int x = 5;
  d->func(x);
  // calls base func
  
  int y = 5;
  d->func(y);
  // calls base func
  
  delete d;
  return 0;
}