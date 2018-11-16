#include <iostream>
#include <string>

// demonstrates '= delete' can be used to get rid of unwanted implicit
// conversions and template instantiations

bool isLucky(int number) { return true; }            // original function

bool isLucky(char) = delete;         // reject chars

bool isLucky(bool) = delete;         // reject bools

bool isLucky(double) = delete;       // reject doubles and
                                     // floats

class Widget {
public:
  template<typename T>
  void processPointer(T* ptr)
  {}
};

template<>
void Widget::processPointer<void>(void*) = delete;

int main() {
  isLucky(1);
  //isLucky(true);
  // compiler: calling explicitly deleted functions
  Widget w1;
  int i = 1;
  void *ptr = (void *)&i;
  //w1.processPointer(ptr);
  // compiler: calling explicitly deleted functions
  return 0;
}