#include <iostream>
#include <string>

// demonstrates pure virtual dtors (a minor use case), and undefined behavior 
// in deleting ChildClass object using a BaseClass pointer whose dtor is not
// virtual

class PureVirtualBase {
  public:
    virtual ~PureVirtualBase() = 0;
};

PureVirtualBase::~PureVirtualBase() {}

class ChildPureVirtualBase : public PureVirtualBase {
  public:
    void print() const { std::cout << "ChildPureVirtualBase\n"; }
};

class PolymorphicBase {
  public:
    PolymorphicBase() : d_x(10) {}
    ~PolymorphicBase() { std::cout << "base class dtor\n"; }
    
    virtual void print() { std::cout << d_x << "\n"; }
  private:
    int d_x;
};

class Child : public PolymorphicBase {
  public:
    Child() : d_y(20) {}
    ~Child() { std::cout << "child class dtor\n"; }

    virtual void print() { std::cout << d_y << "\n"; }
  private:
    int d_y;
};

int main() {
    PureVirtualBase* cp = new ChildPureVirtualBase();
    delete cp;

    PolymorphicBase* c = new Child();
    c->print();
    // undefined behavior, dtor of child will not be called
    delete c;
    // it would appear that nothing happens but expect heap to be corrupted
    return 0;
}