#include <iostream>
#include <string>
#include <boost/type_index.hpp>

// Test forward impl using 
template <typename T>
T&& my_forward(std::remove_reference_t<T>& param) {
  std::cout << "(my_forward) T is deduced to be: "
            << boost::typeindex::type_id_with_cvr<T>().pretty_name()
            << "\n";
  std::cout << "(my_forward) param is deduced to be: "
            << boost::typeindex::type_id_with_cvr<decltype(param)>().pretty_name()
            << "\n";
  return static_cast<T&&>(param);
}

template <typename U>
void func(U&& param) {
  std::cout << "U is deduced to be: "
            << boost::typeindex::type_id_with_cvr<U>().pretty_name()
            << "\n";
  std::cout << "param deduced to be: "
            << boost::typeindex::type_id_with_cvr<decltype(param)>().pretty_name()
            << "\n";

  auto&& forwarded = my_forward<decltype(param)>(param);
  std::cout << "(auto&&) forwarded is deduced to be: "
            << boost::typeindex::type_id_with_cvr<decltype(forwarded)>().pretty_name()
            << "\n";

  auto auto_by_value = my_forward<decltype(param)>(param);
  std::cout << "(auto by value) forwarded is deduced to be: "
            << boost::typeindex::type_id_with_cvr<decltype(auto_by_value)>().pretty_name()
            << "\n";

  std::cout << "\n";
}

int main() {
  int x = 3;
  func(x);
  func(4);
  return 0;
}