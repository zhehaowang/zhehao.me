#include <iostream>
#include <string>

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
  ~Widget() = default;

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
  // Note how they call different overloads
  Widget w;
  auto w1(w);

  const Widget cw;
  auto cw1(cw);

  // Note how child copy ctor calls base's universal ref ctor as opposed to copy ctor
  ChildWidget child;
  auto child1(child);

  return 0;
}