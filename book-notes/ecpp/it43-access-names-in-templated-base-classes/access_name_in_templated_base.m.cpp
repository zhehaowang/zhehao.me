#include <iostream>
#include <string>
#include <memory>

// demonstrates how compiler will not look at functions in templated base by
// default, and three ways of making them do it.

template <typename T>
class Base {
public:
  void log() { std::cout << "log\n"; }
  void log1() { std::cout << "log1\n"; }
private:
  T d_data;
};

template <>
class Base<double> {};

template <typename T>
class Derived : public Base<T> {
public:
  void doStuff() {
    //log();           // use of undeclared identifier 'log'
    this->log();       // fine
    //Base<T>::log();  // fine, but unrecommended, if 'log' is virtual, this
                       // wouldn't employ dynamic binding
  };

  using Base<T>::log1;
  void doStuff1() {
    log1();           // fine, too
  }
};

int main() {
  Derived<int> d;
  d.doStuff();
  return 0;
}