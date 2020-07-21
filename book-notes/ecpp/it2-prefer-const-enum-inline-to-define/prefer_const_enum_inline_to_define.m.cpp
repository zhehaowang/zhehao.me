#include <iostream>
#include <string>

#include <header_with_static_const.h>

// cannot be defined again
// const int MyClass::d_x;

int main() {
    std::cout << "invoking main\n";
    // When replacing #define with consts
    // correct specification of const pointer and const pointed to
    const char* name = "z";
    const char* const name2 = "h";
    const int* const i1 = 0;
    int const * const i2 = 0;

    // the second const does not matter
    const char const* name3 = "e"; // duplicate 'const' declaration specifier
    int const const * i3 = 0; // duplicate 'const' declaration specifier

    // To test class member static const definitions
    MyClass mc;
    mc.d_obj.print();
    std::cout << mc.d_x << " " << mc.d_y << "\n";
    //const int* const addrOfStaticConst = &mc.d_x;
    return 0;
}