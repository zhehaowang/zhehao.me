#include <iostream>
#include <string>

class MyClass {
public:
    MyClass() = default;
    MyClass& operator=(const MyClass&) { std::cout << "cp\n"; return *this; }
    MyClass& operator=(MyClass&&) { std::cout << "mv\n"; return *this; }
private:
    int a;
    int b;
    int c;
};

int main() {
    MyClass mc;
    MyClass temp;
    mc = temp;
    return 0;
}