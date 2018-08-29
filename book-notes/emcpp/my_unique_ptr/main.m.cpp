#include <unique_ptr.h>
#include <string>
#include <iostream>

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
    UniquePtr<CustomData> up;
    cout << "1 isNull: " << up.isNull() << "\n";
    
    up.reset(new CustomData());
    cout << "1 isNull: " << up.isNull() << "\n";
    cout << "1 data: "   << up->x() << "\n";

    up->setX(5);
    cout << "1 data: "   << up->x() << "\n";

    (*up).setX(10);
    cout << "1 data: "   << (*up).x() << "\n";

    // without std::move this calls the deleted copycon by default
    UniquePtr<CustomData> up2 = std::move(up);

    cout << "1 isNull: " << up.isNull() << "\n";
    cout << "2 isNull: " << up2.isNull() << "\n";
    cout << "2 data: "   << up2->x() << "\n";

    return 0;
}