#include <iostream>
#include <string>

// demonstrates the classic template meta programming example: Factorial

// a recursion based factorial
unsigned int factorial(unsigned int n) {
    return n == 0 ? 1 : n * factorial(n - 1);
}

// a tmp based factorial
template <unsigned int n>
struct factorial_tmp {
    enum { value = n * factorial_tmp<n - 1>::value };
};

template <>
struct factorial_tmp<0> {
    enum { value = 1 };
};

int main() {
    std::cout << factorial(4) << "\n";
    // computed at runtime
    
    std::cout << factorial_tmp<4>::value << "\n";
    // computed at compile time
    return 0;
}
