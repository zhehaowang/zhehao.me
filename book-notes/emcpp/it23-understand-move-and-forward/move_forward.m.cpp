#include <iostream>
#include <string>

// demonstrates perfect forwarding with universal references, and the effect of
// std::move / std::forward

class Widget {};

void process(const Widget& lvalArg) {
    std::cout << "called with lvalue\n";
} // process lvalues

void process(Widget&& rvalArg) {
    std::cout << "called with rvalue\n";
} // process rvalues

template<typename T>                     // template that passes
void logAndProcess(T&& param)            // param to process
{
  process(std::forward<T>(param));   // with forward, the expected version
                                     // (dependent upon what's given to param),
                                     // is called.

  //process(std::move<T>(param));    // with move, the rvalue version will
                                     // always be called even if an lvalue is
                                     // given, if an rvalue is given a compiler
                                     // error is thrown

  //process(param);                  // without forward, the lvalue version will
                                     // always be called
}

int main() {
  Widget w;
  logAndProcess(w);
  logAndProcess(Widget());
  return 0;
}