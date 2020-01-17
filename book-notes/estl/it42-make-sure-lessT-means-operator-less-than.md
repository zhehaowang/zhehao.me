### Make sure `less<T>` means `operator<`

`less<T>`, by default, does its work by calling `T::operator<`, one could, however, sever this tie:
```
// a Widget class that can be sorted on two properties
class Widget {
  public:
    ...
    size_t weight() const;
    size_t speed() const;
    ...
};

bool operator<(const Widget& lhs, const Widget& rhs) {
    return lhs.weight() < rhs.weight();
}

template<>
struct std::less<Widget> : public std::binary_function<Widget, Widget, bool> {
    bool operator()(const Widget& lhs, const Widget& rhs) const {
        return lhs.speed() < rhs.speed();
    }
};
```

This is ill advised.
As a general rule trying to modify components in `std` is forbidden but specializing templates in `std` for user defined types is allowed.
An example is `boost::shared_ptr` specializing `std::less` to make boost's version sort like built-in pointers.

C++ programmers are allowed to assume certain things, e.g. copy constructors copy, taking the address of an object yields a pointer to that object (it18 for an example where this isn't true), etc.
Using `less` does the same thing as `operator<` is one such assumption, too.
Having both do different things violate the principle of least surprise.

If we want our `Widget` to be able to sort by `weight()` and `speed()` and `operator<` sorts by `weight()`, don't specialize a `less` and make it sort by `speed()`, make another function object. E.g.
```
struct SpeedCompare : public binary_function<Widget, Widget, bool> {
    bool operator()(const Widget& lhs, const Widget& rhs) const {
        return lhs.speed() < rhs.speed();
    }
};

// use it as
multiset<Widget, SpeedCompare> widgets;

// instead of omitting the second template type and do
multiset<Widget> widgets;
// and specializing less<Widget> to mean something different from operator<
```
