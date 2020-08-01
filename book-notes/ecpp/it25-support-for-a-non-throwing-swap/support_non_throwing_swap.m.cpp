#include <iostream>
#include <string>
#include <vector>

// demonstrates what one should do when needing to provide a class-specific swap
// for efficiency purposes; also how a client should call that swap

namespace something {

class FooImpl {
public:
  FooImpl() : d_x(10), d_v() {}
  FooImpl(int x, const std::vector<int>& v) : d_x(x), d_v(v) {}

  int x() const { return d_x; }
  std::vector<int> v() { return d_v; }
private:
  int d_x;
  std::vector<int> d_v;
};

class Foo {
public:
  void doSomething();

  Foo() : p_impl(new FooImpl()) {}
  
  Foo(const Foo& rhs) {
    // note to self: is there a better way to implement copycon?
    p_impl = new FooImpl(rhs.x(), rhs.v());
  }

  Foo& operator=(const Foo& rhs) {
    // copy over all members of *(rhs.p_impl)
    // std::swap will invoke this twice, making it inefficient in this pimpl
    // class
    *p_impl = *(rhs.p_impl);
    return *this;
  }

  ~Foo() {
    delete p_impl;
  }

  // due to default std::swap being inefficient, we want to add class specific
  // swap
  void swap(Foo& rhs) {
    std::cout << "custom swap called\n";

    FooImpl* temp = rhs.p_impl;
    rhs.p_impl = p_impl;
    p_impl = temp;
  }

  int x() const { return p_impl->x(); }
  std::vector<int> v() const { return p_impl->v(); }
private:
  FooImpl* p_impl;
};

// you should also add a swap within the same namespace of your class
void swap(Foo& lhs, Foo& rhs) {
  std::cout << "something::swap called\n";
  lhs.swap(rhs);
}

}

// you should also specialize the one in std namespace, for misguided clients
// who write swap like std::swap
namespace std {

template<>
void swap<something::Foo>(something::Foo& lhs, something::Foo& rhs) {
  std::cout << "std::swap specialization on Foo called\n";
  lhs.swap(rhs);
}

}

namespace client {

void call_swap_right() {
  something::Foo foo1, foo2;
  // call swap like this, looks up swap in `something` namespace
  // first, then global namespace and finally std 
  using std::swap;
  swap(foo1, foo2);
}

void call_swap_wrong() {
  something::Foo foo1, foo2;
  // not like this, dictates using std
  std::swap(foo1, foo2);
}

}

int main() {
  client::call_swap_right();
  client::call_swap_wrong();
  return 0;
}