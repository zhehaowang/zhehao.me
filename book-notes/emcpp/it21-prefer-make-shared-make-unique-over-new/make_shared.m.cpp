#include <iostream>
#include <string>
#include <unordered_map>

class Widget {
public:
  Widget() = default;
  Widget(const Widget&) = default;
  Widget(Widget&&) = default;
  Widget(int id) : d_x(id) {};

  Widget& operator=(const Widget&) = default;
  Widget& operator=(Widget&&) = default;
private:
  int d_x;
};

void processWidget(std::shared_ptr<Widget>, int priority) {}

int getPriority() {
  throw std::runtime_error("expected");
  return 0;
}

int main() {
  // exception unsafe: in compiler generated code, getPriority can get in between Widget ctor and shared_ptr ctor
  // which will cause memory leak
  try {
    processWidget(std::shared_ptr<Widget>(new Widget()),
                  getPriority());
  } catch (const std::runtime_error&) {
    // swallow
  }

  // exception safe
  try {
    processWidget(std::make_shared<Widget>(),
                  getPriority());
  } catch (const std::runtime_error&) {
    // swallow
  }
  return 0;
}