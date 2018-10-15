#include <iostream>
#include <string>

// demonstrates self-assignment-unsafe and exception-unsafe implementations of operator=
// and ways of making it exception-safe and self-assignment-safe

class Bitmap {
public:
  Bitmap() : d_x(10) {}
  int d_x;
};

class Widget {
public:
  Widget() : d_data(new Bitmap()) {}
  Widget(const Widget& rhs) : d_data(new Bitmap(*rhs.d_data)) {}
  ~Widget() { delete d_data; }

  Widget& operator=(const Widget& rhs);

  void swap(Widget& rhs) {
    std::swap(d_data, rhs.d_data);
    /*
    Bitmap* temp = d_data;
    d_data = rhs.d_data;
    rhs.d_data = temp;
    */
  }
public:
  Bitmap* d_data;
};

Widget& Widget::operator=(const Widget& rhs) {
  // "w = w" should break: dereferencing nullptr
  // in fact it does not
  delete d_data;
  d_data = new Bitmap(*(rhs.d_data));
  return *this;
}

/*
Widget& Widget::operator=(const Widget& rhs) {
  // self-assignment safe, but exception-unsafe
  if (this == &rhs) { return *this; }
  delete d_data;
  d_data = new Bitmap(*(rhs.d_data));
  return *this;
}

Widget& Widget::operator=(const Widget& rhs) {
  // self-assignment safe, exception-safe, reorder statements
  Bitmap* origData = d_data;
  d_data = new Bitmap(*(rhs.d_data));
  delete origData;
  return *this;
}
*/

/*
Widget& Widget::operator=(const Widget& rhs) {
  // self-assignment safe, exception-safe, swap
  Widget temp(rhs);
  swap(temp);
  return *this;
}
*/

int main() {
  Widget w;
  w.d_data->d_x = 15;
  w = w;
  std::cout << w.d_data->d_x << "\n";
  Widget x;
  x = w;
  std::cout << x.d_data->d_x << "\n";

  // With the self-assignment-unsafe and exception-unsafe impl, the program does not break
  return 0;
}