#ifndef INCLUDED_CLIENT_UTILS
#define INCLUDED_CLIENT_UTILS

// note how my_class header is not included

class MyClass;

void printFromMyClass(const MyClass& myClass);
MyClass buildMyClass(MyClass rhs); // this doesn't make sense, but just for demo
                                   // sake

#endif