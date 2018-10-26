#ifndef INCLUDED_MY_CLASS
#define INCLUDED_MY_CLASS

#include <string>

class MyClassImpl;

class MyClass {
  public:
    MyClass();

    ~MyClass() = default;
    MyClass& operator=(const MyClass&) = default;
    MyClass(const MyClass&) = default;

    void doSomething();

    std::string data() const;
  private:
    std::shared_ptr<MyClassImpl> d_impl;
};

#endif