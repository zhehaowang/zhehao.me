#include <iostream>
#include <string>

// demonstrates undefined behavior of dangling reference caused by temporary
// objects + returning a handle to object internal

class MyClass {
public:
  MyClass(const std::string& data) : d_data(data) {}

  const std::string& data() const { return d_data; }
private:
  std::string d_data;
};

MyClass createMyClass(const std::string& data) {
  return MyClass(data);
}

int main() {
  std::string data("data");
  
  const std::string& rd(createMyClass(data).data());
  // rd will be dangling at this point

  MyClass c(createMyClass(data));
  const std::string& rd1(c.data());
  // rd1 will be fine

  std::cout << rd << " " << rd1 << "\n";
  // clang on osx does not demonstrate noticeable behavior for this UB
  return 0;
}