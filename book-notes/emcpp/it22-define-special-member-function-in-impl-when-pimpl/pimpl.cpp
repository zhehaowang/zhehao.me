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
