#include <iostream>
#include <string>

// demonstrates the similarity between template type deduction and auto type
// deduction

int main() {
    const int from = 15;
    // this is like rule 2 in template type deduction.
    // When the given has const but it's pass-by-value, constness is dropped
    auto to = from;
    to += 27;
    std::cout << to << "\n";
    return 0;
}