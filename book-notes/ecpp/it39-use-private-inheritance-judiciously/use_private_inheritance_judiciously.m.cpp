#include <iostream>
#include <string>

// demonstrates (the rare) cases where you might want to consider private
// inheritance: if you want to model "is-implemented-in-terms-of" and you want
// access to protected parts, or redefine virtual functions; or if you care
// about empty base optimization.

class Timer {
public:
  ~Timer() {}

  explicit Timer(int tickFrequency) : d_tickFrequency(tickFrequency) {}
  
  virtual void onTick() const = 0;      // automatically called for each tick
                                        // we want to redefine this
private:
  int d_tickFrequency;
};

class Widget: private Timer {
public:
  explicit Widget() : Timer(10) {}

  void measuredFunc() const { onTick(); }
private:
  virtual void onTick() const { std::cout << "ticks\n"; }
};

// EBO

class Empty {};

class Composition {
public:
  int d_data;
  Empty e;
};

class PrivateInheritance : private Empty {
public:
  int d_data;
};

int main() {
  Widget w;
  w.measuredFunc();

  std::cout << "size of int: "
            << sizeof(int) << "\n";
  std::cout << "size of empty class: "
            << sizeof(Empty) << "\n";
  std::cout << "size of Composition class: "
            << sizeof(Composition) << "\n";
  std::cout << "size of PrivateInheritance class: "
            << sizeof(PrivateInheritance) << "\n";
  return 0;
}