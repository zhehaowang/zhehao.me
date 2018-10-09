#include <my_class.h>

MyClass& getGlobalMyClass() {
    static MyClass myClass;
    return myClass;
}
