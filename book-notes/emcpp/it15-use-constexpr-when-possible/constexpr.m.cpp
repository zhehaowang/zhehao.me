#include <iostream>
#include <string>

// demonstrates a user-defined literal type, whose ctor and other member
// functions are constexpr, and user constexpr functions that work with such
// types. This shifts all the work to compile time.

class Point {
public:
  constexpr Point(double xVal = 0, double yVal = 0) noexcept
  : x(xVal), y(yVal) {
    //std::cout << "ctor\n";
    // compile error: non-constexpr function 'operator<<' cannot be used in a
    // constexpr
  }

  constexpr double xValue() const noexcept { return x; }
  constexpr double yValue() const noexcept { return y; }

  // C++14
  constexpr void setX(double newX) noexcept     // C++14
  { x = newX; }

  constexpr void setY(double newY) noexcept     // C++14
  { y = newY; }
private:
  double x, y;
};

constexpr
Point midpoint(const Point& p1, const Point& p2) noexcept
{
  return { (p1.xValue() + p2.xValue()) / 2,    // call constexpr
           (p1.yValue() + p2.yValue()) / 2 };  // member funcs
}

// and with C++14 (setter constexpr) you can do the following
// return reflection of p with respect to the origin (C++14)
constexpr
Point reflection(const Point& p) noexcept
{
  Point result;                       // create non-const Point

  result.setX(-p.xValue());           // set its x and y values
  result.setY(-p.yValue());

  return result;                      // return copy of it
}

int main() {
  constexpr Point p1(9.4, 27.7);      // fine, "runs" constexpr
                                      // ctor during compilation

  constexpr Point p2(28.8, 5.3);      // also fine
  constexpr auto mid = reflection(midpoint(p1, p2));     // init constexpr

  std::cout << mid.xValue() << ", " << mid.yValue() << "\n";
  // done at compile time

  Point p3(4.9, 5.2);
  p3.setX(10.4);
  std::cout << p3.xValue() << "\n";
  auto p4 = reflection(midpoint(p1, p3));
  std::cout << p4.xValue() << ", " << p4.yValue() << "\n";
  // done at runtime

  return 0;
}