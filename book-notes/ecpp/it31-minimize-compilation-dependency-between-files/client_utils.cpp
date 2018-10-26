#include <client_utils.h>
#include <my_class.h>

#include <iostream>

void printFromMyClass(const MyClass& myClass) {
    std::cout << "a client util whose compilation does not depend on that of "
              << "MyClass : "
              << myClass.data() << "\n";
}

MyClass buildMyClass(MyClass rhs) {
    return MyClass(rhs);
}