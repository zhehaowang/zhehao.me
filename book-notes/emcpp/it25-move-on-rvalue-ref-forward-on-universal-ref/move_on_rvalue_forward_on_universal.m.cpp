#include <iostream>
#include <string>

// demonstrates using std::move on rvalue references (and std::forward) on
// universal references.
// this particular example shows that when returning a reference variable by
// value, you'll want to use std::move if that reference is rvalue reference, or
// std::forward if it's universal reference (move ctor of the returned value
// will be called in the first csae, copy will be called in the second case.)
// don't do this for returning local variables by value though, let rvo take
// care of it for you.

class Widget {
public:
  Widget() = default;
  Widget(const Widget&) noexcept {
    std::cout << "copy ctor\n";
  }
  Widget& operator=(const Widget&) noexcept {
    std::cout << "copy assignment\n";
    return *this;
  }
  Widget(Widget&&) noexcept {
    std::cout << "move ctor\n";
  }
  Widget& operator=(Widget&&) noexcept {
    std::cout << "move assignment\n";
    return *this;
  }
  ~Widget() {
    std::cout << "dtor\n";
  }
};

Widget                                        // by-value return
func(Widget&& w)
{
  return std::move(w);                        // move lhs into
}                                             // return value

Widget                                        // by-value return
func1(Widget&& w)
{
  return w;                        // copy lhs into
}                                  // return value

Widget                                        // by-value return
rvo()
{
  return Widget();                            // don't do std::move or
                                              // std::forward. let rvo take care
                                              // of this
}     

int main() {
  //Widget x = func(Widget());
  Widget x = func1(Widget());
  return 0;
}