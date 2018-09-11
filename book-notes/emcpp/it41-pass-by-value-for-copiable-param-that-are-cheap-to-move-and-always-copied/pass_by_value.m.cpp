#include <iostream>
#include <string>
#include <vector>

using namespace std::literals;

class Widget {
public:
  void addName_byValue(std::string value) {
    m_vec.push_back(std::move(value));
  }

  template <typename T>
  void addName_universalRef(T&& value) {
    // would using T here be the same as using decltype(value)?
    // what happens with std::forward<std::string&&>?
    m_vec.push_back(std::forward<T>(value));
  }

  void addName_ref(const std::string& value) {
    m_vec.push_back(value);
  }

  void addName_ref(std::string&& value) {
    m_vec.push_back(std::move(value));
  }
private:
  std::vector<std::string> m_vec;
};

int main() {
  Widget w;
  std::string str("bad stuff");
  
  // 2 moves
  w.addName_byValue("good stuff");
  // 1 copy 1 move
  w.addName_byValue(str);

  // 1 move
  w.addName_ref("good stuff");
  // 1 copy
  w.addName_ref(str);

  // 1 move? const char* is forwarded the ctor the string to be then moved into the vector
  // if emplace is used, this would be 0 copy / move? 
  w.addName_universalRef("good stuff");
  // 1 copy
  w.addName_universalRef(str);
  return 0;
}