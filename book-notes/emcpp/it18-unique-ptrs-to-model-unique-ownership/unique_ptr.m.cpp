#include <iostream>
#include <string>

// demonstrates using unique_ptr with custom deleter as a factory method return
// type.

const auto CREATE_TYPE = 1;

class Investment {
public:
  virtual ~Investment() = default;
};

class Stock : public Investment {
public:
  Stock() {
    std::cout << "default ctor Stock\n";
  }
  ~Stock() {
    std::cout << "dtor Stock\n";
  }
};

class Bond : public Investment {
public:
  Bond() {
    std::cout << "default ctor Bond\n";
  }
  ~Bond() {
    std::cout << "dtor Bond\n";
  }
};

class RealEstate : public Investment {
public:
  RealEstate() {
    std::cout << "default ctor RealEstate\n";
  }
  ~RealEstate() {
    std::cout << "dtor RealEstate\n";
  }
};

// Example with variadic template and custom unique_ptr deleter
auto delInvmt = [](Investment* pInvestment)       // custom
                {                                 // deleter
                  std::cout << "custom dtor\n";   // (a lambda
                  delete pInvestment;             // expression)
                };

template<typename... Ts>                          // revised
std::unique_ptr<Investment, decltype(delInvmt)>   // return type
// can return auto in C++14
makeInvestment(Ts&&... params)
{
  std::unique_ptr<Investment, decltype(delInvmt)> // ptr to be
    pInv(nullptr, delInvmt);                      // returned

  if (CREATE_TYPE == 1) {
    pInv.reset(new Stock(std::forward<Ts>(params)...));
  }
  else if (CREATE_TYPE == 2) {
    pInv.reset(new Bond(std::forward<Ts>(params)...));
  }
  else {
    pInv.reset(new RealEstate(std::forward<Ts>(params)...));
  }

  return pInv;
}


int main() {
  auto investment = makeInvestment();
  return 0;
}