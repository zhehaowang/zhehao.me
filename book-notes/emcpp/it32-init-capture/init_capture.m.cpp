#include <iostream>
#include <string>
#include <vector>

class Widget {
public:
  Widget(int x) : d_x(x) {}
  void doStuff() const noexcept { std::cout << d_x << "\n"; }
private:
  int d_x;
};

// What a closure looks like (context: imitating C++14 init
// capture with C++11)
class WidgetDoStuff {
public:
  using DataType = std::unique_ptr<Widget>;

  explicit WidgetDoStuff(DataType&& ptr)      // Item 25 explains
  : pw(std::move(ptr)) {}                     // use of std::move

  void operator()() const { pw->doStuff(); }

private:
  DataType pw;
};

int main() {
// Test of C++14 init capture
  auto func = [widget = std::make_unique<Widget>(3)]() { widget->doStuff(); };

  std::vector<int> v = {1, 2, 3};

  std::cout << "(C++14) vector moved into closure:\n";
  [x = std::move(v)](){
    for (const auto& s: x) {
      std::cout << s << "\n";
    }
  }();

  std::cout << "(C++14) vector outside after move:\n";
  for (const auto& s: v) {
    std::cout << s << "\n";
  }

// Imitate init capture with bind in C++11
  std::cout << "(C++11) vector moved into closure:\n";
  std::vector<int> v1 = {4, 5, 6};
  auto func1 = std::bind(
    [](std::vector<int>& v) mutable {
      v.push_back(7);
      for (const auto& s: v) {
        std::cout << s << "\n";
      }
    },
    std::move(v1));
  func1();

  std::cout << "(C++11) vector outside after move:\n";
  for (const auto& s: v1) {
    std::cout << s << "\n";
  }

  auto w1 = WidgetDoStuff(std::make_unique<Widget>(5));
  w1();
  return 0;
}