#include <iostream>
#include <string>

// demonstrates overloading a function taking in universal references will
// almost always end up with the overload taking in universal references called
// more often than expected.
// in this case ctor universal reference overloading is particularly
// problematic.

class Widget {
public:
  Widget() = default;
  Widget(const Widget&) noexcept {
    std::cout << "base copy ctor\n";
  }
  Widget& operator=(const Widget&) noexcept {
    std::cout << "base copy assignment\n";
    return *this;
  }
  Widget(Widget&&) noexcept {
    std::cout << "base move ctor\n";
  }
  Widget& operator=(Widget&&) noexcept {
    std::cout << "base move assignment\n";
    return *this;
  }
  virtual ~Widget() = default;

  template <typename T>
  Widget(T&& rhs) noexcept {
    std::cout << "base universal reference ctor\n";
  }
};

class ChildWidget : public Widget {
public:
  ChildWidget() = default;
  ChildWidget(const ChildWidget& rhs) : Widget(rhs) {
    std::cout << "child copy ctor\n";
  }
  ChildWidget(ChildWidget&& rhs) : Widget(std::move(rhs)) {
    std::cout << "child move ctor\n";
  }
};

int main() {
  // note how they call different overloads
  Widget w;
  auto w1(w);  // calls perfect forwarding ctor, which does not require adding
               // const as copy ctor does

  const Widget cw;
  auto cw1(cw); // calls copy ctor, since when a template and a regular function
                // are equally good matches, regular function wins

  // note how child copy ctor calls base's universal ref ctor as opposed to copy
  // ctor, reasoning being that when calling the parent class's ctor, an object
  // of type Child is passed in.
  ChildWidget child;
  auto child1(child);

  return 0;
}