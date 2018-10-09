#ifndef INCLUDED_MY_CLASS
#define INCLUDED_MY_CLASS

#include <iostream>
#include <string>

// we want a translation-unit scope instance of MyClass and DependsOnMyClass, and 
// we want to make sure MyClass is always instantiated first

class MyClass {
  public:
    MyClass() : d_x(10) {
        std::cout << "MyClass ctor\n";
    }

    int d_x;
};

class DependsOnMyClass {
  public:
    DependsOnMyClass(int y) : d_y(y) {
        std::cout << "DependsOnMyClass ctor\n";
    }

    int d_y;
};


DependsOnMyClass& getGlobalDependsOnMyClass();

MyClass& getGlobalMyClass();

#endif