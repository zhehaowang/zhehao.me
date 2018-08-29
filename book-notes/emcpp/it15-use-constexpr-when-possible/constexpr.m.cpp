#include <iostream>
#include <string>

class Point {
public:
  constexpr Point(double xVal = 0, double yVal = 0) noexcept
  : x(xVal), y(yVal)
  {}

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
  // all done at compile time

  return 0;
}