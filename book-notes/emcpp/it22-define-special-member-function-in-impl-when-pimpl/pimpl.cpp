#include <pimpl.h>

#include <iostream>
#include <string>
#include <vector>

struct Widget::Impl {
  std::vector<int> d_x;
  int d_y;
};

void Widget::doStuff() const {
  std::cout << p_impl->d_y << "\n";
}

Widget::Widget() : p_impl(std::make_unique<Widget::Impl>()) {}

Widget::~Widget() = default;
// we still need to define this even if it's using default, here we define it so
// that we don't run into linker errors.

Widget::Widget(Widget&& rhs) noexcept = default;              
Widget& Widget::operator=(Widget&& rhs) noexcept = default;
// you can't do "= default" in the header, since that would make it a definition
// and to be able to generate code for the default move (move assignment needs
// to destroy the previously managed item; move ctor needs to be able to delete
// Impl in case of an exception, even though you declare it noexcept), Impl
// needs to be a complete type

Widget::Widget(const Widget& rhs) : p_impl(nullptr) {
  if (rhs.p_impl) {
    p_impl = std::make_unique<Impl>(*rhs.p_impl);
  }
}

Widget&
Widget::operator=(const Widget& rhs)
{
  if (!rhs.p_impl) {
    p_impl.reset();
  } else if (!p_impl) {
    p_impl = std::make_unique<Impl>(*rhs.p_impl);
  } else {
    *p_impl = *rhs.p_impl;
  }
  
  return *this;
}
