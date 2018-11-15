#include <iostream>
#include <string>

// demonstrates how unscoped enum names are leaked to the scope containing its
// definition

int main() {
  enum State { good, bad };
  //int good = 2; // compiler error: redefinition
  enum class ScopedState : char { good, good1, bad };
  int good1 = 2;
  //MyList<int> list2;
  return 0;
}