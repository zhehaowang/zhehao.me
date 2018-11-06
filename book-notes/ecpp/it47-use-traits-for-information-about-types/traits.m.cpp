#include <iostream>
#include <string>
#include <memory>

// demonstrates using traits to represent information about classes, with
// iterator_traits as an example

// tag as type names
struct my_random_access_iterator {};
struct my_bidirectional_iterator {};

// we introduce iterator_traits class template such that we have a unified way
// to refer to the Iter's iterator_category, no matter if the Iter's a built-in
// type (pointer type partial specialization below), or if Iter's a user defined
// type (the parrot back typedef here)
template <typename Iter>
struct iterator_traits {
  typedef typename Iter::Iterator::iterator_category iterator_category;
};

template <typename Iter>              // partial template specialization
struct iterator_traits<Iter*>         // for built-in pointer types
{
  typedef my_random_access_iterator iterator_category;
};

// MyVector wants a random access iterator.
// Here we give iterator_traits something to parrot back.
template <typename T>
class MyVector {
public:
  struct Iterator {
    typedef my_random_access_iterator iterator_category;
  };
};

// MyList wants a bidirectional iterator
template <typename T>
class MyList {
public:
  struct Iterator {
    typedef my_bidirectional_iterator iterator_category;
  };
};

// this shows using type as a parameter, through overload with type as a
// parameter, we at compile time knows which impl to call, for different traits.
template <typename Iter, typename D>
void advance(Iter iter, D d) {
  doAdvance(iter, d, typename iterator_traits<Iter>::iterator_category());
  // note how we use iterator_traits<Iter> (instead of Iter::Iterator) to refer
  // to the iterator_category, this is the unified way described in
  // iterator_traits definition. If we do use Iter::Iterator::iterator_category
  // we wouldn't need iterator_traits template, but we also wouldn't be able to
  // accommodate pointers being random_access_iterators.
}

template <typename Iter, typename D>
void doAdvance(Iter iter, D d, my_random_access_iterator) {
  std::cout << "random access iterator advance\n";
}

template <typename Iter, typename D>
void doAdvance(Iter iter, D d, my_bidirectional_iterator) {
  std::cout << "bidirectional iterator advance\n";
}

int main() {
  MyVector<int> myv;
  advance(myv, 5);

  MyList<int> myl;
  advance(myl, 10);

  void* p = nullptr;
  advance(p, 2);

  return 0;
}