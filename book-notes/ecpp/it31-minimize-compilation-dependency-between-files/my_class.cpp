#include <my_class.h>
#include <my_class_impl.h>

#include <iostream>
#include <string>
#include <memory>

void MyClassImpl::doSomething() {
    std::cout << "data content: " << d_data << "\n";
}

void MyClass::doSomething() {
    d_impl->doSomething();
}

std::string MyClass::data() const {
    return d_impl->d_data;
}

MyClass::MyClass() : d_impl(std::make_shared<MyClassImpl>()) {}

MyClassImpl::MyClassImpl() : d_data("default") {}