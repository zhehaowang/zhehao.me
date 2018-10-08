#include <header_with_static_const.h>

#include <iostream>
#include <string>

const ConstObject MyClass::d_obj; // fine, invokes default ctor before main

const int MyClass::d_x = 5;

void ConstObject::print() const {
    std::cout << d_c << "\n";
}