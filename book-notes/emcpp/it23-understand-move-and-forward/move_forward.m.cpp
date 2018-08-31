#include <iostream>
#include <string>

class Widget {
public:
  
};

void process(const Widget& lvalArg) {
    std::cout << "called with lvalue\n";
} // process lvalues

void process(Widget&& rvalArg) {
    std::cout << "called with rvalue\n";
} // process rvalues

template<typename T>                     // template that passes
void logAndProcess(T&& param)            // param to process
{
  process(std::forward<T>(param)); // without forward, the lvalue version will always be called
  //process(std::move<T>(param));    // with move, the rvalue version will always be if an lvalue is given,
                                     // if an rvalue is given a compiler error is thrown
  //process(param);                  // with forward, the expected version (dependent upon what's
                                     // given to param), is called
}

int main() {
  Widget w;
  logAndProcess(w);
  logAndProcess(Widget());
  return 0;
}