#include <iostream>
#include <string>

#include <boost/type_index.hpp>

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
  return 0;
}