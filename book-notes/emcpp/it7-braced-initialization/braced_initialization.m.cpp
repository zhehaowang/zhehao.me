#include <iostream>
#include <string>
#include <vector>

using namespace std;

class Widget {
  public:
    Widget(long double a, bool b) { cout << "ctor1\n"; };
    Widget(std::initializer_list<float> list) { cout << "ctor initializer_list\n"; };
  private:
};

int main() {
  Widget w1(12.0, true);
  Widget w2{12.0, true};
  long double x = 0;
  //Widget w3{x, true};
  // compiler would complain about long double narrowing, as opposed to trying ctor1

  // std::initializer_list ctor in action
  std::vector<int> vec1(1, 2);
  std::vector<int> vec2{1, 2};
  
  cout << "\n";
  for (auto p: vec1) {
    cout << "vec1:" << p << "\n";
  }
  
  cout << "\n";
  for (auto p: vec2) {
    cout << "vec2:" << p << "\n";
  }
  return 0;
}