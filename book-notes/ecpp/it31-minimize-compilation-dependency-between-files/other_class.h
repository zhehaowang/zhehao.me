#ifndef INCLUDED_OTHER_CLASS
#define INCLUDED_OTHER_CLASS

#include <memory>
#include <string>

// this interface exists to achieve the same thing pimpl achieves: making
// compilation depend on declaration rather than definition.
// in its definition the client is locked in with the ConcreteOtherClass, making
// it not a typical Strategy-pattern-like implementation.

class OtherClass {
  public:
    virtual ~OtherClass() {}
    virtual void doSomething() = 0;
    virtual std::string data() const = 0;

    static std::unique_ptr<OtherClass> createOtherClass();
};

#endif