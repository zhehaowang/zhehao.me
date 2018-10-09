#include <my_class.h>

DependsOnMyClass& getGlobalDependsOnMyClass() {
    static DependsOnMyClass dependsOnMyClass(getGlobalMyClass().d_x);
    return dependsOnMyClass;
}
