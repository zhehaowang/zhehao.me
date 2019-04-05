#include <iostream>
#include <string>
#include <memory>
#include <vector>

// Say we want to end up copying from src (an object MyClass) to dst (an object
// NyContainer that holds a member MyClass) if src is an lvalue, and moving
// from src to dst if src is an rvalue. What is a good way to do that?

class MyClass {
  public:
    MyClass() {
        std::cout << "default ctor\n";
    }

    MyClass(const MyClass& lhs) {
        std::cout << "cp ctor\n";
    }

    MyClass(MyClass&& lhs) {
        std::cout << "mv ctor\n";
    }

    ~MyClass() {
        std::cout << "dtor\n";
    }

    MyClass& operator=(const MyClass& lhs) = default;
    MyClass& operator=(MyClass&& lhs) = default;
};

class MyContainer {
  public:
    // pass by value then mv.
    // when given lvalue, this copies into param then param moves into dst
    // when given rvalue, this moves  into param then param moves into dst
    // while one mv can be saved, it may not be worth the effort (it41, emcpp)
    MyContainer(MyClass lhs) : _lhs(std::move(lhs)) {}
  private:
    MyClass _lhs;
};

class MyContainer2 {
  public:
    // this maintains two copies of the function (alternatively use a universal
    // reference but that comes with its own implications e.g. appearing in
    // header as a template), but saves one move when given lvalue or rvalue.
    MyContainer2(const MyClass& lhs) : _lhs(std::move(lhs)) {}
    MyContainer2(MyClass&& lhs) : _lhs(std::move(lhs)) {}
  private:
    MyClass _lhs;
};

class MyContainer3 {
  public:
    MyContainer3(std::vector<std::unique_ptr<MyClass>>&& lhs) : _lhs(std::move(lhs)) {}
  private:
    std::vector<std::unique_ptr<MyClass>> _lhs;
};

int main() {
    std::cout << "1:\n";
    MyClass m;
    MyContainer c1(m);
    std::cout << "2:\n";
    MyClass m1;
    MyContainer c2(std::move(m1));
    std::cout << "3:\n";
    // {}-init for most vexing parse
    MyContainer c3{MyClass()};

    std::cout << "4:\n";
    MyClass m2;
    MyContainer2 c4(m2);
    std::cout << "5:\n";
    MyClass m3;
    MyContainer2 c5(std::move(m3));
    std::cout << "6:\n";
    MyContainer2 c6{MyClass()};

    std::cout << "7\n";
    std::vector<std::unique_ptr<MyClass>> vec;
    vec.emplace_back(std::make_unique<MyClass>());
    MyContainer3 m4(std::move(vec));
    return 0;
}