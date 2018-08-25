#include <iostream>
#include <string>
#include <vector>

// test 1
decltype(auto) f1()
{
  int x = 0;
  return x;        // decltype(x) is int, so f1 returns int
}

decltype(auto) f2()
{
  int x = 0;
  return (x);      // decltype((x)) is int&, so f2 returns int&
                   // this emits a compiler warning
}

// test 2
template<typename Container, typename Index>    // C++14;
auto authAndAccess(Container& c, Index i)       // not quite
{                                               // correct
  return c[i];                  // return type deduced from c[i]
                                // using template type deduction rules
                                // (rule 3, by value)
}

template<typename Container, typename Index>
auto& authAndAccess1(Container& c, Index i)
{
  return c[i];                  // return type deduced from c[i]
                                // using template type deduction rules
                                // (rule 1, non-universal reference / pointer)
}

template<typename Container, typename Index>
decltype(auto) authAndAccess2(Container&& c, Index i)
{
  return std::forward<Container>(c)[i]; // The correct version
}

int main() {
  // test 1
  f2() = 4;        // this would result in undefined behavior

  // test 2
  std::vector<int> v;
  v.push_back(1);
  //authAndAccess(v, 0) = 1; // would not compile
  authAndAccess1(v, 0) = 1;
  authAndAccess2(v, 0) = 1;
  return 0;
}