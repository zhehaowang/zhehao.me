#include <iostream>
#include <string>
#include <my_class.h>

// demonstrates enforcing certain order in initializing non-local static variables in different translation units

int main() {
  std::cout << "main called\n";
  std::cout << getGlobalDependsOnMyClass().d_y << "\n";
  return 0;
}