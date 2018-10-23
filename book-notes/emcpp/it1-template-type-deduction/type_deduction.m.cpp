#include <iostream>
#include <string>

// demonstates rule 1 and rule 2 in template type deduction, and that a function
// template parameter expecting an lvalue (reference) cannot work with a given
// rvalue

template <typename T>
void func_lvalue_reference(T& param) {
    std::cout << param << "\n";
}

template <typename T>
void func_universal_reference(T&& param) {
    std::cout << param << "\n";
}

int main() {
    int x = 42;
    
    // cannot pass compile: template expects a lvalue reference
    //func_lvalue_reference(27);

    // works fine
    func_lvalue_reference(x);
    func_universal_reference(27);
    func_universal_reference(x);

    const int& crx = x;
    func_lvalue_reference(crx);
    func_universal_reference(crx);
    
    return 0;
}