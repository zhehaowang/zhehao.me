#include <iostream>
#include <string>

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

int main() {
  Widget x = func1(Widget());
  return 0;
}