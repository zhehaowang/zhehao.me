#ifndef INCLUDED_CONCRETE_OTHER_CLASS
#define INCLUDED_CONCRETE_OTHER_CLASS

#include <other_class.h>

class ConcreteOtherClass : public OtherClass {
  public:
    ConcreteOtherClass();
    virtual void doSomething();
    virtual std::string data() const;
  private:
    std::string d_data;
};

#endif