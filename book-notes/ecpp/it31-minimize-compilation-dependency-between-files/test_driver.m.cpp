// demonstrates using pimpl to reduce compile time dependency (depend on
// declarations as opposed to definitions)

#include <client_utils.h>

// client caller is not aware of the impl header (using pimpl)
#include <my_class.h>

// client caller is not aware of derived impl class header (using inheritance)
#include <other_class.h>

#include <iostream>
#include <string>
#include <memory>

int main() {
  MyClass obj;
  obj.doSomething();

  MyClass obj1(buildMyClass(obj));
  printFromMyClass(obj1);

  std::unique_ptr<OtherClass> obj2 = OtherClass::createOtherClass();
  obj2->doSomething();

  return 0;
}