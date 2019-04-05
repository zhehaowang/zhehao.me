#include <iostream>
#include <string>

class Base {
public:
    int x;
};

class A : public Base {};

class B : public Base {};

// Note where your virtual inheritance should be: this doesn't help!
class D : virtual public A, virtual public B {
public:
    // void setX(int _x) { x = _x; }
    // compile error! ambiguous!
};

class E : virtual public Base {};

class F : virtual public Base {};

class G : public E, public F {
public:
    void setX(int _x) { x = _x; }
};

int main() {
    D d;
    // d.setX(3);
    G g;
    g.setX(3);
    return 0;
}