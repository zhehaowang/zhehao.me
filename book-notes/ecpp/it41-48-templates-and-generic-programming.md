# Templates and generic programming

### Understand implicit interfaces and compile-time polymorphism

We are familiar with explicit interfaces (one that is explicitly visible in the source code), and runtime polymorphism (via virtual function).

In the world of templates and generic programming, implicit interface and compile time polymorphism move to the fore.

Consider this code
```cpp
template<typename T>
void doProcessing(T& w) {
  if (w.size() > 10 && w != someNastyWidget) {
     T temp(w);
     temp.normalize();
     temp.swap(w);
  }
}
```
`w`'s type `T` must support `size`, `normalize` and `swap`, copyctor, and comparison for inequality.
While these aren't yet complete, but they suffice for now to demonstrate the **implicit interface** `T` must support.

To make calls involving `w` succeed we need to instantiate the template at compile time.
Because instantiating function templates with different template parameters leads to different functions being called, this is known as **compile time polymorphism**.

An implicit interface is not based on function signatures, rather, it consists of valid expressions: e.g. `T` must often a `size` member function that returns an integral.
Actually not necessarily an integral, rather, as long as `size()` returns a type `X` that has `operator<(Y, int)` defined, where `X` can implicit convert to `Y`, then we can instantiate this call.

Just as you can't use an object in a way contradictory to the explicit interface its class offers (the code won't compile), you can't try to use an object in a template unless that object supports the implicit interface the template requires (again, the code won't compile).

**Takeaways**
* Both classes and templates support interfaces and polymorphism
* For classes, interfaces are explicit and centered on function signatures. Polymorphism occurs at runtime through virtual functions
* For template parameters, interfaces are implicit and based on valid expressions. Polymorphism occurs during compilation through template instantiation and function overloading resolution

### Understand the two meanings of `typename`

These two mean the same thing:
```cpp
template<class T> class Widget;                 // uses "class"
template<typename T> class Widget;              // uses "typename"
```

Suppose that we want to print the 2nd element of an STL compliant container, with a template function looking like this (which won't compile but just for illustration purposes)
```cpp
template<typename C>                            // print 2nd element in
void print2nd(const C& container)               // container;
{                                               // this is not valid C++!
  if (container.size() >= 2) {
     C::const_iterator iter(container.begin()); // get iterator to 1st element
     ++iter;                                    // move iter to 2nd element
     int value = *iter;                         // copy that element to an int
     std::cout << value;                        // print the int
  }
}
```
`iter`'s type `C::const_iterator` depends on the template parameter `C`, they are called dependent names.
`const_iterator` is nested inside a class, making it a nested dependent type name.

Nested dependent names can be difficult for parsing.
Suppose this
```cpp
template<typename C>
void print2nd(const C& container)
{
  C::const_iterator * x;
  ...
}
```
We know that `C::const_iterator` is a type name, but the compiler doesn't.
What if `const_iterator` is a static member of `C`, and `x` is a global variable? Then the above wouldn't be declaring a pointer, but rather a multiplication of two values.

Until `C` is known, there is no way to know whether `C::const_iterator` is a type or not, and when the template `print2nd` is parsed, `C` is not known.
This is resolved with this rule: if the parser encounters a nested dependent name in a template, it assumes the name is not a type name unless you tell it otherwise.

By default, nested dependent names are not types.
This is why the first snippet fails: `C::const_iterator` is not considered a type by the compiler.
We have to tell the compiler it is a type, by putting `typename` right before it:
```cpp
template<typename C>                           // this is valid C++
void print2nd(const C& container) {
  if (container.size() >= 2) {
    typename C::const_iterator iter(container.begin());
    ...
  }
}
```

The rule is simple, any time you refer to a nested dependent name in a template, you must precede it by the word `typename`.
But only do it for nested dependent type names:
```cpp
template<typename C>                   // typename allowed (as is "class")
void f(const C&             container,   // typename not allowed
       typename C::iterator iter);       // typename required
```

There is one exception to the rule, `typename` must not precede nested dependent type names in a list of base classes or as a base class identifier in a member initialization list.
E.g.
```cpp
template<typename T>
class Derived: public Base<T>::Nested { // base class list: typename not
public:                                 // allowed
  explicit Derived(int x)
  : Base<T>::Nested(x)                  // base class identifier in mem
  {                                     // init. list: typename not allowed

    typename Base<T>::Nested temp;      // use of nested dependent type
    ...                                 // name not in a base class list or
  }                                     // as a base class identifier in a
  ...                                   // mem. init. list: typename required
};
```

Another example on `typename`:
```cpp
template<typename IterT>
void workWithIterator(IterT iter) {
  typename std::iterator_traits<IterT>::value_type temp(*iter);
  ...
}
```
This makes a copy of what the iterator points to in `temp`.
Type of `temp` is the same as what `iter` points to. (E.g. if `IterT` is `vector<int>::iterator`, `temp` is of type `int`)

Many programmers `typedef` this entire thing
```cpp
template<typename IterT>
void workWithIterator(IterT iter) {
  typedef typename std::iterator_traits<IterT>::value_type value_type;
  value_type temp(*iter);
  ...
}
```
You may find this `typedef typename` weird, but it follows from the rule of putting `typename` before nested dependent type names.

Finally, it's worth pointing out that compilers differ in enforcing this rule: some accept code where `typename` is required but missing, some accept `typename` being present but not allowed, and some reject `typename` where it's required.
This could cause minor portability headaches.

**Takeaways**
* When declaring template parameters, class and typename are interchangeable
* Use typename to identify nested dependent type names, except in base class lists or as a base class identifier in a member initialization list

### Know how to access names in templatized base classes

Suppose we have this code
```cpp
class CompanyA {
public:
  ...
  void sendCleartext(const std::string& msg);
  void sendEncrypted(const std::string& msg);
  ...
};

class CompanyB {
public:
  ...
  void sendCleartext(const std::string& msg);
  void sendEncrypted(const std::string& msg);
  ...
};
...                                     // classes for other companies

class MsgInfo { ... };                  // class for holding information
                                        // used to create a message
template<typename Company>
class MsgSender {
public:
  ...                                   // ctors, dtor, etc.

  void sendClear(const MsgInfo& info)
  {
    std::string msg;
    create msg from info;

    Company c;
    c.sendCleartext(msg);
  }

  void sendSecret(const MsgInfo& info)   // similar to sendClear, except
  { ... }                                // calls c.sendEncrypted
};
```
This will work fine, when instantiating `MsgSender` with `CompanyA` or `CompanyB`.

Now suppose we want some extra behaviors in `MsgSender`, say, logging the message, we can achieve this with a derived class.
```cpp
template<typename Company>
class LoggingMsgSender: public MsgSender<Company> {
public:
  ...                                    // ctors, dtor, etc.
  void sendClearMsg(const MsgInfo& info) {
    // write "before sending" info to the log;

    sendClear(info);                     // call base class function;
                                         // this code will not compile!
    // write "after sending" info to the log;
  }
  ...
};
```
Now this won't compile.
Standard compliant compilers will complain that `sendClear` does not exist, it's in the base class, but compilers won't look for it there, because when compiler encounter the definition of `LoggingMsgSender`, they know the base class is `MsgSender<Company>`, but this template won't be instantiated until later.
Without knowing what `Company` is, there is no way to know if `MsgSender<Company>` has a `sendClear` function.

Suppose we have a `CompanyZ` which only sends encrypted, and a total specialization for `CompanyZ` to accommodate that.
```cpp
class CompanyZ {                             // this class offers no
public:                                      // sendCleartext function
  ...
  void sendEncrypted(const std::string& msg);
  ...
};

template<>                                 // a total specialization of
class MsgSender<CompanyZ> {                // MsgSender; the same as the
public:                                    // general template, except
  ...                                      // sendCleartext is omitted
  void sendSecret(const MsgInfo& info)
  { ... }
};
```
The `template <>` at the beginning signifies this is neither a template, nor a standalone class.
Rather, it's a specialized version of the template `MsgSender`, when the template argument is `CompanyZ`.

This is known as **total template specialization**.

Now consider `LoggingMessageSender`, if `Company` is `CompanyZ`, there will be no such `sendClear` to call.

The compiler knows with a total specialization, the function in templated base may not exist, so it refuses to look inside templated base for that function.

There are three ways to disable this "don't look inside templated base" behavior.
You can preface the call with `this`
```cpp
template<typename Company>
class LoggingMsgSender: public MsgSender<Company> {
public:
  void sendClearMsg(const MsgInfo& info) {
    ...
    this->sendClear(info);                // okay, assumes that
                                          // sendClear will be inherited
    ...
  }
};
```
You can employ a `using` declaration
```cpp
template<typename Company>
class LoggingMsgSender: public MsgSender<Company> {
public:
  using MsgSender<Company>::sendClear;   // tell compilers to assume
  ...                                    // that sendClear is in the
                                         // base class
  void sendClearMsg(const MsgInfo& info) {
    ...
    sendClear(info);                   // okay, assumes that
    ...                                // sendClear will be inherited
  }
  ...
};
```
A final way is to explicitly specify the function you are calling
```cpp
template<typename Company>
class LoggingMsgSender: public MsgSender<Company> {
public:
  ...
  void sendClearMsg(const MsgInfo& info) {
    ...
    MsgSender<Company>::sendClear(info);      // okay, assumes that
    ...                                       // sendClear will be
  }                                           //inherited
  ...
};
```
This is generally the least favored behavior, because if the function you are calling is `virtual`, you'd be turning off the virtual binding behavior.

From a name visibility point of view, the three do the same thing: promise compiler that any subsequent specialization of base class template will support the interface offered by the general template.

If the compiler later finds out that this promise is not satisfied:
```cpp
LoggingMsgSender<CompanyZ> zMsgSender;
MsgInfo msgData;
zMsgSender.sendClearMsg(msgData);            // error! won't compile
```
An error will be emitted.

**Takeaways**
* In derived class templates, refer to names in base class templates via a “this->” prefix, via using declarations, or via an explicit base class qualification

### Factor parameter independent code out of templates

Templates let you save time and avoid code replication, but if you are not careful, using templates can lead to code bloat: the source may look trim and fit, but the binary is fat and flabby.

The primary way to avoid this is commonality and variability analysis, the same thing you do when you extract the shared code out of two code paths and put them in one function that both calls.

Only that with templates, the replication is implicit.
E.g. consider this code
```cpp
template<typename T,           // template for n x n matrices of
         std::size_t n>        // objects of type T; a non-type parameter
class SquareMatrix {           // on the size_t parameter
public:
  ...
  void invert();              // invert the matrix in place
};

SquareMatrix<double, 5> sm1;
...
sm1.invert();                  // call SquareMatrix<double, 5>::invert

SquareMatrix<double, 10> sm2;
...
sm2.invert();                  // call SquareMatrix<double, 10>::invert
```
Two copies of similar `invert` will be generated as a result, one works with 5, and another with 10.
How about parameterizing `size` instead?
```cpp
template<typename T>                   // size-independent base class for
class SquareMatrixBase {               // square matrices
protected:
  ...
  void invert(std::size_t matrixSize); // invert matrix of the given size
  ...
};

template<          typename T, std::size_t n>
class SquareMatrix: private SquareMatrixBase<T> {
private:
  using SquareMatrixBase<T>::invert;   // avoid hiding base version of
                                       // invert; see Item 33
public:
  ...
  void invert() { this->invert(n); }   // make inline call to base class
};                                     // version of invert; see below
                                       // for why "this->" is here
```
This version, there will be only one version of `invert` logic generated in `SquareMatrixBase`.

Additional cost of function call `base->invert` should be 0: it's inlined.
Also note the `this->` as otherwise template base function won't be visible.
Finally note the `private` inheritance, meaning "is-implemented-in-terms-of", not "is-a".

How does the base know what data to invert on? We could have it hold a pointer to matrix values and the matrix size.
Like this
```cpp
template<typename T>
class SquareMatrixBase {
protected:
  SquareMatrixBase(std::size_t n, T *pMem)     // store matrix size and a
  : size(n), pData(pMem) {}                    // ptr to matrix values

  void setDataPtr(T *ptr) { pData = ptr; }     // reassign pData
  ...

private:
  std::size_t size;                            // size of matrix

  T *pData;                                    // pointer to matrix values
};

```
And let the derived classes decide how to allocate memory
```cpp
template<typename T, std::size_t n>
class SquareMatrix: private SquareMatrixBase<T> {
public:
  SquareMatrix()                             // send matrix size and
  : SquareMatrixBase<T>(n, data) {}          // data ptr to base class
  ...

private:
  T data[n*n];
};
```
This parameterized version comes at a cost: the size-specific would run faster, as e.g. the sizes would be compile-time constants, hence eligible for optimizations such as constant propagation, which can't be done in size-independent version.

On the other hand, having one version of `invert` means smaller binary, and better locality of reference in instruction cache.

To decide which of the above effects would dominate requires trying both out.

Type parameters can lead to bloat, too.
E.g. on many platforms `int` and `long` have the same binary representation, so `vector<int>` and `vector<long>` would be identical, the very definition of bloat.
Some linkers would merge those identical function implementations, some will not causing bloats.

Similarly, on most platforms all pointer types have the same binary representation, so templates holding pointer types (e.g. `list<int*>`, `list<const int*>`) should often be able to use a single underlying implementation for each member function.
Typically, this means implementing member functions that work with untyped pointers (`void*`).
Some implementations of the standard library do this for templates like `vector`, `deque` and `list`. (Could this be a reason why `bslma::ManagedPtr` template underlying is a `void*`?).
If you are concerned with code bloat, you could do the same thing.

**Takeaways**
* Templates generate multiple classes and multiple functions, so any template code not dependent on a template parameter causes bloat
* Bloat due to non-type template parameters can often be eliminated by replacing template parameters with function parameters or class data members
* Bloat due to type parameters can be reduced by sharing implementations for instantiation types with identical binary representations

### Use member function templates to accept all compatible types

