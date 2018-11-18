#include <iostream>
#include <string>
#include <algorithm>

// demonstrates the contigency of std::swap's noexcept on user type's noexcept,
// and that you can declare an obviously throwing function noexcept, which would
// violate correctness.

class Point {
public:
  Point(double xVal = 0, double yVal = 0) noexcept
  : x(xVal), y(yVal)
  {}

  double xValue() const noexcept { return x; }
  double yValue() const noexcept { return y; }

  void setX(double newX) noexcept { x = newX; }
  void setY(double newY) noexcept { y = newY; }

private:
  double x, y;
};

int func() noexcept {
    throw std::runtime_error("declared noexcept but actually throws");
    // this would generate a compiler warning
    return 0;
}

int main() {
  int x[2][3];
  int y[2][3];

  Point p1, p2;

  using std::swap;
  // swap's noexcept is contingent upon the noexcept-ness of the given
  // parameters.
  // noexcept() tests for the noexceptness of a call
  std::cout << noexcept(swap(x, y)) << "\n";
  std::cout << noexcept(swap(p1, p2)) << "\n";
  // can I make swap on two points not noexcept?

  try {
    func();
    // note how this is not caught: correctness is violated due to the presence
    // of a wrongful noexcept
  } catch (const std::runtime_error& ex) {
    std::cout << "caught: " << ex.what() << "\n";
  }
  
  return 0;
}