### Make predicates pure functions

A **predicate** is a function that returns `bool` or something implicitly convertable to `bool`.
STL uses predicates a lot, e.g. `find_if` takes a predicate, comparisons for standard associative containers are also predicates.

A **pure function** is a function whose return value depends only on its parameters.

A **predicate class** is a functor class whose `operator()` function is a predicate.
Any place where a predicate is expected, an object of a predicate class will also do.

Algorithms may make copies of functors and hold on to them for a while before using them.
A critical repercussion of this observation is that predicate functions must be pure functions.

E.g. this badly designed predicate class
```
class BadPredicate : public unary_function<Widget, bool> {
  public:
    BadPredicate(): timesCalled(0) {}
    bool operator()(const Widget&) {
        return ++timesCalled == 3;
    }
  private:
    size_t timesCalled;
};

// suppose we use this BadPredicate to eliminate the third Widget from vector
vector<Widget> w;
// fill some stuff
w.erase(remove_if(w.begin(), w.end(), BadPredicate()), w.end());
```

This looks reasonable but with many STL implementations it will not only remove the third, but also the sixth.

Imagine `remove_if` being implemented this way
```
template <typename ForwardIterator, typename Predicate>
ForwardIterator remove_if(ForwardIterator begin,
                          ForwardIterator end,
                          Predicate p) {
    begin = find_if(begin, end, p);
    if (begin == end) {
        return begin;
    } else {
        ForwardIterator next = begin;
        return remove_copy_if(++next, end, begin, p);
    }
}
```

As `find_if` makes a copy of `p` and modifies its state instead, when `remove_copy_if` copies a `p`, the `p.timesCalled` is still 0.
This causes us to remove the third as well as the sixth element.

The easiest way to keep yourself from tumbling into this is to declare `operator()` `const` in predicate classes.
```
class BadPredicate : public unary_function<Widget, bool> {
  public:
    BadPredicate(): timesCalled(0) {}
    bool operator()(const Widget&) const {
        return ++timesCalled == 3;        // error! can't modify the value
    }
  private:
    size_t timesCalled;
};
```

We could just follow the simple rule of "make `operator()` `const` in predicate classes".
But note that `const` member functions can still modify mutable data members, non-const local static objects, non-const class static objects, non-const objects at namespace scope, and non-const global objects.
A well-designed predicate class ensures that its `operator()` functions are independent of those kinds of objects, too.

So making `operator()` of predicates `const` is necessary but not sufficient (similarly, this is necessary but not sufficient for a function to be a pure function).

This restriction applies to predicate functions, too.
(Anywhere STL expects a predicate object, a predicate function (possibly modified by `ptr_fun`) will also work).
So this is bad, too:
```
bool badPredicateFunction(const Widget&, const Widget&) {
    static int timesCalled = 0;
    return ++timesCalled == 3;
}
```

Regardless how you write your predicates, they should always be pure functions.
