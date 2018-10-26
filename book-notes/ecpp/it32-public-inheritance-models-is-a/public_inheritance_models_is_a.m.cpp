#include <iostream>
#include <string>

// demonstrates 

class Base {};

class Derived : public Base {};

void expectBase(Base x) {}

int main() {
  Derived d;
  expectBase(d);
  return 0;
}