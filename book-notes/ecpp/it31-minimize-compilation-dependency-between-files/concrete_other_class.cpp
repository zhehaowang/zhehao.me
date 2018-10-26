#include <concrete_other_class.h>
#include <iostream>

ConcreteOtherClass::ConcreteOtherClass() : d_data("good") {}

void ConcreteOtherClass::doSomething() {
    std::cout << "other class uses inheritance to hide definition : "
              << d_data << "\n";
}

std::string ConcreteOtherClass::data() const {
    return d_data;
}

std::unique_ptr<OtherClass> OtherClass::createOtherClass() {
    return std::make_unique<ConcreteOtherClass>();
}

