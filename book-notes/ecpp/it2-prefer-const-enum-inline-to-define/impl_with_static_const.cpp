#include <header_with_static_const.h>

#include <iostream>
#include <string>

const ConstObject MyClass::d_obj; // fine, invokes default ctor before main

const int MyClass::d_x;

void ConstObject::print() const {
    std::cout << d_c << "\n";
}
