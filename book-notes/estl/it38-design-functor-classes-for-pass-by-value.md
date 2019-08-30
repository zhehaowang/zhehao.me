# Design functor classes for pass-by-value

In C/C++ you can pass a function using pointers to functions

Standard library `qsort` looks like

```
void qsort(
    void *base,
    size_t nmemb,
    size_t size,
    int (*cmpfcn)(const void*, const void*));
```

Note that the `cmpfcn` function pointer is copied from the call site to `qsort`: passed by value.

STL function objects are modeled after function pointers, so the convention is STL function objects are, too, passed by value.
E.g. standard's declaration of `for_each`

```
template <class InputIterator, class Function>
Function for_each(InputIterator first, InputIterator last, Function f);
// f passes by value, and for_each returns by value
```

You could explicitly specify the parameter type so it's not mandatory that `for_each` passes value.
E.g. the following passes by reference:

```
class DoSomething : public unary_function<int, void> {
  public:
    void operator()(int x) { ... }
};

deque<int> di;
DoSomething d;

for_each<deque<int>::iterator, DoSomething&>(di.begin(), di.end(), d);
// this forces pass functor by reference and return by reference
```

Users almost never do the above though, and for STL implementation this won't compile.
So in practice, function objects are almost always passed by value.

So the responsibility falls on you to make sure yuor function objects behave well when copied.
This means two things:
* You want them to be small otherwise they'd be too expensive to copy.
* They must not be polymorphic: must not use virtual functions. This is because derived class objects passed by value into parameters of base class type suffer from slicing.

In practice you may want a function object with lots of states or polymorphic.
In which case you can move it to a different class and give your functor a pointer to this class.

```
// you want a functor to do this
template <typename T>
class BigPolymorphicFunctorClass : public unary_function<T, void> {
  private:
    Widget w;
    int x;
    ... // lots of data, and don't want to be passed by value
  public:
    virtual void operator()(const T& val) const; // virtual function, don't want
                                                 // slicing
};

// could do it via the pimpl below
template <typename T>
class BigPolymorphicFunctorClassImpl {
  private:
    Widget w;
    int x;
    ...
    virtual ~BigPolymorphicFunctorClassImpl(); // polymorphic class need virtual 
                                               // dtors
    virtual void operator()(const T& val) const;
    
    friend class BigPolymorphicFunctorClass<T>;
};

template <typename T>
class BigPolymorphicFunctorClass : public unary_function<T, void> {
  private:
    BigPolymorphicFunctorClassImpl<T> *pImpl;  // now small, only the size of *
  public:
    void operator()(const T& val) const {      // now monomorphic, forwards to
        pImpl->operator()(val);                // pImpl
    }
};

```
pimpl idiom, bridge pattern, etc, are related.

You'll need to make sure `BigPolymorphicFunctorClass`'s copycon does something reasonable, perhaps you'll want `pImpl` as a `shared_ptr`.
