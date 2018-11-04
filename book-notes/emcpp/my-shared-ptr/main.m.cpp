#include <string>
#include <iostream>

#include <shared_ptr.h>

using namespace std;

class CustomData {
  public:
    CustomData() {
        cout << "default ctor\n";
    }
    ~CustomData() {
        cout << "dtor\n";
    }

    CustomData& operator=(const CustomData&) = default;
    CustomData(const CustomData&) = default;

    CustomData& operator=(CustomData&&) = default;
    CustomData(CustomData&&) = default;

    void doStuff() const {
        cout << "print " << d_x << "\n";
    }

    int x() const noexcept {
        return d_x;
    }

    void setX(int x) noexcept {
        d_x = x;
    }
  private:
    int d_x;
};

class Derived : public CustomData {};

int main() {
    Derived *x = new Derived();
    x->setX(5);
    SharedPtr<Derived> sp(x); // bad style
    SharedPtr<Derived> sp1(sp);

    SharedPtr<Derived> sp2(new Derived());
    sp2->setX(10);
    sp1 = sp2;
    sp  = sp2;
    
    // TODO: fix this after we add generalized move / copy ctor support
    //SharedPtr<CustomData> bp1(SharedPtr<Derived>(new Derived()));
    return 0;
}