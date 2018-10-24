#include <iostream>
#include <string>
#include <unordered_map>
#include <boost/type_index.hpp>

using namespace std;

// demonstrates small missteps in declaring a type by hand (pair of a map) could
// lead to inefficiency in code, whereas auto avoids such issues

// test 1
class KeyClass {
  public:
    explicit KeyClass() { std::cout << "ctor1\n"; }
    explicit KeyClass(int x) : d_x(x) { std::cout << "ctor2\n"; }
    KeyClass(const KeyClass& rhs) { std::cout << "copycon\n"; }
    
    bool operator==(const KeyClass& rhs) const {
      return d_x == rhs.x();
    }
    
    int x() const { return d_x; }
  private:
    int d_x;
};

namespace std {
  template <>
  struct hash<KeyClass> {
    std::size_t operator()(const KeyClass& k) const {
      return hash<int>()(k.x());
    }
  };
}

int main() {
  std::unordered_map<KeyClass, int> map;
  map.insert(make_pair(3, 4));
  map.insert(make_pair(4, 5));
  map.insert(make_pair(5, 6));

  // note how copycon is called 3 times here
  // (also 'explicit' on copycon will cause compiler error)
  for (const std::pair<KeyClass, int>& p : map) {
    // p here is a compiler constructed temporary, if you get its address, it's not gonna
    // pointer to an element in map
    cout << "elem: " << p.second << "\n";
  }

  // note how copycon is not called here, since
  // p is std::pair<const KeyClass, int>
  for (auto p : map) {
    cout << "type of p: " << boost::typeindex::type_id_with_cvr<decltype(p)>() << "\n";
    cout << "elem: " << p.second << "\n";
  }

  // cout << "type of map iterator: " << boost::typeindex::type_id_with_cvr<std::unordered_map<KeyClass, int>::iterator>() << "\n";
  // cout << "type of map iterator: " << boost::typeindex::type_id_with_cvr<decltype(map.begin())>() << "\n";
  return 0;
}