#include <iostream>
#include <string>

class A {
public:
    virtual ~A() {};
};

class B : public A {
public:
    ~B() override {};
};

class C : public A {
public:
    ~C() override {};
};

int main() {
    B* b = new B();
    C* c = dynamic_cast<C*>(b);
    if (c) {
        std::cout << "c is good\n";
    }
    return 0;
}