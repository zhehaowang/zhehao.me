#include <string>
#include <iostream>

class ConstObject {
  public:
    ConstObject() : d_c(10) {
        std::cout << "ConstObject default ctor\n";
    };

    int d_c;

    void print() const;
};

class MyClass {
  public:
    static const int d_x = 5; // fine until you ask for its address,
                              // in which case, define it like d_obj in impl file, leave the initialization here
                              // and only do "const int MyClass::d_x" in the impl file
    static const ConstObject d_obj; // needs definition, correct way to define it is inside the impl file
    static const std::string d_s; // does not need definition due to no usage
};

//ConstObject MyClass::d_obj; // wrong, double definition