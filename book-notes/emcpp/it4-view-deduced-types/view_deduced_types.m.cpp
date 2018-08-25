#include <iostream>
#include <string>
#include <vector>
#include <boost/type_index.hpp>

// test 1
class Widget {
  public:
    int x;
};

std::vector<Widget> createVec() {    // factory method
  return std::vector<Widget>();
}

// to trigger a compiler error to display type
template<typename T>       // declaration only for TD;
class TD;                  // TD == "Type Displayer"


template<typename T>
void f(const T& param)
{
  std::cout << "T =     " << typeid(T).name() << '\n';     // show T

  std::cout << "param = " << typeid(param).name() << '\n'; // show
                                                           // param's
                                                           // type
  // at runtime both are reported as PK6Widget, pointer to const
  // Widget (character-length-6): const Widget*
  // which could be incorrect as it should const (const Widget*)&.

  //TD<T> p1;
  //TD<decltype(param)> p1;
  // TD spits the right type "const Widget *const &"

  using boost::typeindex::type_id_with_cvr;
  // show T
  std::cout << "T =     "
            << type_id_with_cvr<T>().pretty_name()
            << '\n';

  // show param's type
  std::cout << "param = "
            << type_id_with_cvr<decltype(param)>().pretty_name()
            << '\n';
  // at runtime boost::typeindex spits the right types
}

int main() {
  const auto vw = createVec();         // init vw w/factory return

  f(&vw[0]);                           // call f
  return 0;
}