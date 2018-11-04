#include <unique_ptr.h>
#include <string>
#include <iostream>

using namespace std;

class CustomData {
  public:
    CustomData() : d_x(-1) {
        cout << "default ctor\n";
    }
    virtual ~CustomData() {
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
    UniquePtr<Derived> up;
    cout << "1 isNull: " << up.isNull() << "\n";
    
    up.reset(new Derived());
    cout << "1 isNull: " << up.isNull() << "\n";
    cout << "1 data: "   << up->x() << "\n";

    up->setX(5);
    cout << "1 data: "   << up->x() << "\n";

    (*up).setX(10);
    cout << "1 data: "   << (*up).x() << "\n";

    // without std::move this calls the deleted copycon by default
    UniquePtr<Derived> up2 = std::move(up);

    cout << "1 isNull: " << up.isNull() << "\n";
    cout << "2 isNull: " << up2.isNull() << "\n";
    cout << "2 data: "   << up2->x() << "\n";

    UniquePtr<Derived> up3 = UniquePtr<Derived>(new Derived());
    up3->setX(15);
    cout << "3 data: "   << up3->x() << "\n";

    UniquePtr<CustomData> cd = up3;
    cout << "3 isNull: " << up3.isNull() << "\n";
    cout << "parent data: " << cd->x() << "\n";

    UniquePtr<CustomData> cd1 = UniquePtr<Derived>(new Derived());
    cout << "parent1 data: " << cd1->x() << "\n";

    return 0;
}