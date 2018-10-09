#include <iostream>
#include <string>

// demonstrates a case where compiler will not generate copy assignment operator for you
class MyClass {
  public:
    MyClass(int x, int y) : d_x(x), d_y(y) {}

    MyClass& operator=(const MyClass& rhs) = default;
    // this would still compile but does nothing, it appears
  private:
    const int d_x;
    int d_y;
};

int main() {
    MyClass m1(3, 4);
    MyClass m2(5, 6);
    //m1 = m2; // implicit copy assignment opr not defined 
    return 0;
}