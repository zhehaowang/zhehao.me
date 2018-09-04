# Chap 6 Lambda Expressions

Everything a lambda can do is something you can do by hand with a bit more typing. But lambdas are such a convenient way to create function objects, the impact on day-to-day C++ software development is enormous.

Without lambdas, the STL “\_if” algorithms (e.g., std::find\_if, std::remove\_if, std::count\_if, etc.) tend to be employed with only the most trivial predicates, but when lambdas are available, use of these algorithms with nontrivial conditions blossoms.

Outside the STL, lambdas make it possible to quickly create custom deleters for std::unique\_ptr and std::shared\_ptr.

Beyond the Standard Library, lambdas facilitate the on-the-fly specification of callback functions, interface adaption functions, and context-specific functions for one-off calls.

A **lambda expression** that looks like
```cpp
std::find_if(container.begin(), container.end(),
             [](int val) { return 0 < val && val < 10; });
```

A **closure** is the runtime object created by a lambda.
Depending on the capture mode, closures hold copies of or references to the captured data.
In the call to std::find\_if above, the closure is the object that’s passed at runtime as the third argument to std::find\_if.

A **closure class** is a class from which a closure is instantiated.
Each lambda causes compilers to generate a unique closure class.
The statements inside a lambda become executable instructions in the member functions of its closure class.

Closures may generally be copied, so it's usually possible to have multiple closures of a closure type corresponding to a single lambda. E.g. in the following
```cpp
{

  int x;                                 // x is local variable
  …
  auto c1 =                              // c1 is copy of the
    [x](int y) { return x * y > 55; };   // closure produced
                                         // by the lambda

  auto c2 = c1;                          // c2 is copy of c1

  auto c3 = c2;                          // c3 is copy of c2
  …
}
```

In C++14, you can use auto in in lambda parameter specifications.

### Avoid default capture modes

There are two default capture modes in C++11: by-reference and by-value. Default by-reference capture can lead to dangling references.
Default by-value capture lures you into thinking you’re immune to that problem (you’re not), and it lulls you into thinking your closures are self-contained (they may not be).

A by-reference capture causes a closure to contain a reference to a local variable or to a parameter that’s available in the scope where the lambda is defined.
If the lifetime of a closure created from that lambda exceeds the lifetime of the local variable or parameter, the reference in the closure will dangle. E.g.
```cpp
using FilterContainer =                     // see Item 9 for
  std::vector<std::function<bool(int)>>;    // "using", Item 5
                                            // for std::function

FilterContainer filters;                    // filtering funcs

// add a filter for multiples of 5 like
filters.emplace_back(                       // see Item 42 for
  [](int value) { return value % 5 == 0; }  // info on
);                                          // emplace_back

// if we don't want to hardcode the 5, the code would look like
void addDivisorFilter()
{
  auto calc1 = computeSomeValue1();
  auto calc2 = computeSomeValue2();

  auto divisor = computeDivisor(calc1, calc2);

  filters.emplace_back(                              // danger!
    [&](int value) { return value % divisor == 0; }  // ref to
  );                                                 // divisor
                                                     // will
                                                     // dangle!
}
// UB: lambda refers to local var divisor, which ceases to exist when addDivisorFilter returns.
// THe function added to the vector is then basically dead on arrival.

// With explicit captures, same problem:
filters.emplace_back(
  [&divisor](int value)                // danger! ref to
  { return value % divisor == 0; }     // divisor will
);                                     // still dangle!

```

If you know a closure will be used immediately (e.g. being passed to an STL algorithm) and won't be copied, there is no risk that references it holds will outlive the local variables and parameters in the environment where its lambda is created.
But long term, it's simply better software engineering to explicitly list the local variables and parameters that a lambda depends on.

One way to solve the problem with divisor would a by-value capture mode.
```cpp
filters.emplace_back(                                 // now
  [=](int value) { return value % divisor == 0; }     // divisor
);                                                    // can't
                                                      // dangle
```
However by value capture might not work for pointer parameters in which case you copy a pointer and outside they might delete the pointer.

Consider the following code
```cpp
class Widget {
public:
  …                                  // ctors, etc.
  void addFilter() const;            // add an entry to filters

private:
  int divisor;                       // used in Widget's filter
};

// addFilter could be defined like this:
void Widget::addFilter() const
{
  filters.emplace_back(
    [=](int value) { return value % divisor == 0; }
  );
}
```
This looks safe, but is not: captures only apply to non-static local variables (including parameters) visible in the scope where the lambda is created.
In the body of Widget::addFilter, divisor is not a local variable but a data member of Widget. It cannot be captured.
Yet if you leave out the default capture mode, or explicitly put divisor to be captured, the capture won't compile.
```cpp
  filters.emplace_back(
    [divisor](int value)                // error! no local
    { return value % divisor == 0; }    // divisor to capture
  );

  filters.emplace_back(                             // error!
    [](int value) { return value % divisor == 0; }  // divisor
  );                                                // not
```
So what are we capturing and how do we make it safe?
The explanation hinges on the implicit use of a raw pointer this: inside Widget member function, compiler replaces divisor with this->divisor, and in the first version of Widget::addFilter the default by-value capture captures Widget's this pointer, not divisor.
Like this:
```cpp
void Widget::addFilter() const
{
  auto currentObjectPtr = this;
  
  filters.emplace_back(
    [currentObjectPtr](int value)
    { return value % currentObjectPtr->divisor == 0; }
  );
}
```

Thus the viability of the added closure from this lambda is tied to the lifetime of the Widget whose this pointer it obtained a copy of.
Consider this code:
```cpp
using FilterContainer =                     // as before
  std::vector<std::function<bool(int)>>;

FilterContainer filters;                    // as before

void doSomeWork()
{
  auto pw =                       // create Widget; see
    std::make_unique<Widget>();   // Item 21 for
                                  // std::make_unique

  pw->addFilter();                // add filter that uses
                                  // Widget::divisor
  …
}                                 // destroy Widget; filters
                                  // now holds dangling pointer!
```

This particular problem can be solved by making a local copy of the data member you want to capture then capturing the copy:
```cpp
void Widget::addFilter() const
{
  auto divisorCopy = divisor;                // copy data member

  filters.emplace_back(
    [divisorCopy](int value)                 // capture the copy
    { return value % divisorCopy == 0; }     // use the copy
  );
}
```
Default capture mode with just [=] would work as well, but why tempt fate? A default capture mode is what made it possible to accidentally capture this pointer when you thought it's capturing divisor.

In C++14, a better way to capture a data member is to use generalizd lambda capture:
```cpp
void Widget::addFilter() const
{
  filters.emplace_back(               // C++14:
    [divisor = divisor](int value)    // copy divisor to closure
    { return value % divisor == 0; }  // use the copy
  );
}
```

An additional drawback to default by-value captures is that they can suggest that the corresponding closures are self-contained and insulated from changes to data outside the closures.
In general, that’s not true, because lambdas may be dependent not just on local variables and parameters (which may be captured), but also on objects with static storage duration.
Such objects are defined at global or namespace scope or are declared static inside classes, functions, or files.
These objects can be used inside lambdas, but they can’t be captured.
Yet specification of a default by-value capture mode can lend the impression that they are.

Consider this code:
```cpp
void addDivisorFilter()
{
  static auto calc1 = computeSomeValue1();      // now static
  static auto calc2 = computeSomeValue2();      // now static

  static auto divisor =                         // now static
    computeDivisor(calc1, calc2);

  filters.emplace_back(
    [=](int value)                     // captures nothing!
    { return value % divisor == 0; }   // refers to above static
  );

  ++divisor;                           // modify divisor
}
```
In here nothing is captured: the lambda refers to the static variable divisor.
Thus the lambdas pushed into filters will exhibit new behaviors as the static divisor changes.

If you stay away from default by-value capture clauses, you eliminate the risk of your code being misread in this way.

**Takeaways**
* Default by-reference capture can lead to dangling references.
* Default by-value capture is susceptible to dangling pointers (especially "this" pointer), and it misleadingly suggests that lambdas are self-contained.

### Use init capture to move objects into closures

