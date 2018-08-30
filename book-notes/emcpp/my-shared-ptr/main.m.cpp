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


int main() {
    CustomData *x = new CustomData();
    x->setX(5);
    SharedPtr<CustomData> sp(x); // bad style
    SharedPtr<CustomData> sp1(sp);

    SharedPtr<CustomData> sp2(new CustomData());
    sp2->setX(10);
    sp1 = sp2;
    sp  = sp2;
    
    cout << "initial tests\n";
    return 0;
}