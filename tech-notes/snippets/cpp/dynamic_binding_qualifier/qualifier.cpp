#include <iostream>
#include <string>

class A {
public:
    virtual ~A() = default;
    virtual int get() { return 3; }
private:
};

class B : public A {
public:
};

class C : public B {
public:
    virtual int get() { return 4; }
};

int main() {
    C* c = new C();
    std::cout << c->B::get() << "\n";
    delete c;
    return 0;
}