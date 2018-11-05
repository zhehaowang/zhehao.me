#include <iostream>
#include <string>
#include <memory>

// demonstrates how to support mixed-mode operations (implicit conversion) (item
// 24) when template classes come into play:
// when writing a class template that offers functions related to the template
// that support implicit type conversions on all parameters, define those
// functions as friends inside the class template

template <typename T>
class Rational {
  public:
    Rational(T p = 1, T q = 1) : d_p(p), d_q(q) {}

    T p() const { return d_p; }
    T q() const { return d_q; }

    friend
    Rational operator*(const Rational& lhs, const Rational& rhs) {
      return Rational(lhs.p() * rhs.p(), lhs.q() * rhs.q());
    }
    // note that the 'friend' here has nothing to do with being able to access
    // private parts of Rational, rather, to make this a non-member function
    // such that (essentially) implicit conversions on 'this' pointer would be
    // considered.
    // And if we declare this friend we have to define it: either do it like
    // this, or have this call a templated version defined outside. (You can't
    // forego the definition here / a call, since the implementation of the
    // templated version outside won't be instantiated automatically)
  private:
    T d_p;
    T d_q;
};

int main() {
  Rational<int> s(1, 2);

  Rational<int> t = s * 2;
  std::cout << t.p() << "/" << t.q() << "\n";

  t = 2 * s;
  std::cout << t.p() << "/" << t.q() << "\n";
  return 0;
}