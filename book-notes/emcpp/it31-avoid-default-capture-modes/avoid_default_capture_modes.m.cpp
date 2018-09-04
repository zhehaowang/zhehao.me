#include <iostream>
#include <string>
#include <vector>

using FilterContainer = std::vector<std::function<bool(int)>>;

class Widget {
public:
  Widget(int x) : d_x(x) {};

  void setX(int x) noexcept {
    d_x = x;
  }

  void appendFilter(FilterContainer& filters) {
    // this doesn't capture d_x, it captures 'this'!
    filters.push_back([=](int value) {
      return value % d_x == 0;
    });
  }

  void appendFilter_safe(FilterContainer& filters) {
    // this doesn't capture d_x, it captures 'this'!
    filters.push_back([d_x = d_x](int value) {
      return value % d_x == 0;
    });

    // alternatively, captures by value
    int x = d_x;
    filters.push_back([x](int value) {
      return value % x == 0;
    });
  }
private:
  int d_x;
};

void unique_ptr_ub(FilterContainer& container) {
  auto w1 = std::make_unique<Widget>(3);
  w1->appendFilter(container);
}

int main() {
// Test that changing data member value affects the added closure's behavior
  FilterContainer container;
  Widget w(3);
  w.appendFilter(container);

  std::cout << container[0](4) << "\n";
  w.setX(4);  // this changes the behavior of the pushed closure
  std::cout << container[0](4) << "\n";

// Test that deallocating 'this' results in UB
  FilterContainer container1;
  unique_ptr_ub(container1);

  std::cout << container1[0](4) << "\n";
  // undefined behavior as the Widget allocated by the unique_ptr
  // in the call has its 'this' go out of scope
  std::cout << container1[0](3) << "\n";
  // we don't directly observe anything uncommon here...
  return 0;
}