#include <iostream>
#include <string>

#include <boost/type_index.hpp>

// demonstrates the two preconditions for a reference to be considered universal
// type deduction and the form T&&. (template <typename T> T&&, or auto&& t)

template <typename T>
class Widget {
public:
  void rvalueReferenceBinding(T&& x) {
    std::cout << "rvalue reference binding "
              << boost::typeindex::type_id_with_cvr<decltype(x)>().pretty_name()
              << "\n";
  }

  template<typename U>
  void universalReferenceBinding(U&& y) {
    std::cout << "universal reference binding "
              << boost::typeindex::type_id_with_cvr<decltype(y)>().pretty_name()
              << "\n";
    y = 3;
  }
};

int main() {
  Widget<int> w;
  int b = 4;
  w.rvalueReferenceBinding(3);
  // cannot bind lvalue to something expecting rvalue reference
  //w.rvalueReferenceBinding(b);
  
  w.universalReferenceBinding(b);
  w.universalReferenceBinding(2);
  //std::cout << b << "\n";
  return 0;
}