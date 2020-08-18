# Templates and generic programming

### Item 41. Understand implicit interfaces and compile-time polymorphism

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

An implicit interface is not based on function signatures, rather, it consists of valid expressions: e.g. `T` must have a `size` member function that returns an integral.
Actually not necessarily an integral, rather, as long as `size()` returns a type `X` that has `operator<(Y, int)` defined, where `X` can implicit convert to `Y`, then we can instantiate this call.

Just as you can't use an object in a way contradictory to the explicit interface its class offers (the code won't compile), you can't try to use an object in a template unless that object supports the implicit interface the template requires (again, the code won't compile).

**Takeaways**
* Both classes and templates support interfaces and polymorphism
* For classes, interfaces are explicit and centered on function signatures. Polymorphism occurs at runtime through virtual functions
* For template parameters, interfaces are implicit and based on valid expressions. Polymorphism occurs during compilation through template instantiation and function overloading resolution

### Item 42. Understand the two meanings of `typename`

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

### Item 43. Know how to access names in templatized base classes

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
* In derived class templates, refer to names in base class templates via a `this->` prefix, via using declarations, or via an explicit base class qualification

### Item 44. Factor parameter independent code out of templates

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
  void invert();               // invert the matrix in place
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

template<typename T, std::size_t n>
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
Some implementations of the standard library do this for templates like `vector`, `deque` and `list`.
`bslma::ManagedPtr` template underlying, `bslma::ManagedPtr_Members` does not use a template and uses `void*` instead, out of the same concern.
If you are concerned with code bloat, you could do the same thing.

**Takeaways**
* Templates generate multiple classes and multiple functions, so any template code not dependent on a template parameter causes bloat
* Bloat due to non-type template parameters can often be eliminated by replacing template parameters with function parameters or class data members
* Bloat due to type parameters can be reduced by sharing implementations for instantiation types with identical binary representations

### Item 45. Use member function templates to accept all compatible types

Iterators into STL containers are almost always smart pointers.

Real pointers do well in supporting implicit conversion, and emulating such behaviors in smart pointers can be tricky.
We want the following to compile:
```cpp
class Top { ... };
class Middle: public Top { ... };
class Bottom: public Middle { ... };

template<typename T>
class SmartPtr {
public:                             // smart pointers are typically
  explicit SmartPtr(T *realPtr);    // initialized by built-in pointers
  ...
};

SmartPtr<Top> pt1 =                 // convert SmartPtr<Middle> ⇒
  SmartPtr<Middle>(new Middle);     //   SmartPtr<Top>

SmartPtr<Top> pt2 =                 // convert SmartPtr<Bottom> ⇒
  SmartPtr<Bottom>(new Bottom);     //   SmartPtr<Top>

SmartPtr<const Top> pct2 = pt1;     // convert SmartPtr<Top> ⇒
                                    //  SmartPtr<const Top>
```
Compiler will view `SmartPtr<Middle>` and `SmartPtr<Top>` as different classes, to get the conversion we want we have to program them explicitly.

There is no way for to write out all ctors we need. Though all we need is a ctor template:
```cpp
template<typename T>
class SmartPtr {
public:
  template<typename U>                       // member template
  SmartPtr(const SmartPtr<U>& other);        // for a "generalized
  ...                                        // copy constructor"
};
```
Ctors like this (create one object from another object whose type is a different instantiation of the same template) are known as **generalized copy ctors**.
We want to allow implicit conversion so this ctor is not explicit.

We'll want to restrict the relationship between `T` and `U`.
```cpp
template<typename T>
class SmartPtr {
public:
  template<typename U>
  SmartPtr(const SmartPtr<U>& other)         // initialize this held ptr
  : heldPtr(other.get()) { ... }             // with other's held ptr

  T* get() const { return heldPtr; }
  ...

private:                                     // built-in pointer held
  T *heldPtr;                                // by the SmartPtr
};
```
Note the `heldPtr` initialization, this will compile only if there is an implicit conversion from a `U*` to `T*`, and that's what we want.

Another common role for member function templates is to support assignment. E.g. `std::shared_ptr`
```cpp
template<class T> class shared_ptr {
public:
  template<class Y>                                     // construct from
    explicit shared_ptr(Y * p);                         // any compatible
  template<class Y>                                     // built-in pointer,
    shared_ptr(shared_ptr<Y> const& r);                 // shared_ptr,
  template<class Y>                                     // weak_ptr, or
    explicit shared_ptr(weak_ptr<Y> const& r);          // unique_ptr
  template<class Y>
    explicit shared_ptr(unique_ptr<Y>&& r);

  template<class Y>                                     // assign from
    shared_ptr& operator=(shared_ptr<Y> const& r);      // any compatible
  template<class Y>                                     // shared_ptr or
    shared_ptr& operator=(unique_ptr<Y>&& r);            // unique_ptr
  ...
};
```
Note that among the ctors only the generalized copy ctor is not explicit, meaning one can't implicit convert `weak_ptr`, raw pointer or `unique_ptr` to a `shared_ptr`, but implicit conversions among `shared_ptr`s is allowed.

Also note that in the version with `unique_ptr`, the given pointer is not `const` since ownership is taken over when copying an `unique_ptr`.

Declaring a generalized copy constructor (a member template) in a class doesn't keep compilers from generating their own copy ctor (a non-template), so if you want to control all aspects of copy construction, you must declare both a generalized copy ctor as well as the normal copycon.

E.g. excerpt in `std::shared_ptr`
```cpp
template<class T> class shared_ptr {
public:
  shared_ptr(shared_ptr const& r);                 // copy constructor

  template<class Y>                                // generalized
    shared_ptr(shared_ptr<Y> const& r);            // copy constructor

  shared_ptr& operator=(shared_ptr const& r);      // copy assignment

  template<class Y>                                // generalized
    shared_ptr& operator=(shared_ptr<Y> const& r); // copy assignment
  ...
};
```

_Refer to [my_unique_ptr](../emcpp/my-unique-ptr) for code example._

**Takeaways**
* Use member function templates to generate functions that accept all compatible types
* If you declare member templates for generalized copy construction or generalized assignment, you'll still need to declare the normal copy constructor and copy assignment operator, too

### Item 46. Define non-member functions inside templates when type conversions are desired

Recall that item 24 explained why non-member functions are eligible for implicit type conversions on all arguments (`operator*` on `Rational` class, why it should not be a member function, instead, have a non-member so that `lhs` is eligible for implicit conversion.)

Consider this code, goal is to support the same mixed mode operation item 24 pointed out.
```cpp
template<typename T>
class Rational {
public:
  Rational(const T& numerator = 0,     // see Item 20 for why params
           const T& denominator = 1);  // are now passed by reference

  const T numerator() const;           // see Item 28 for why return
  const T denominator() const;         // values are still passed by value,
  ...                                  // Item 3 for why they're const
};

template<typename T>
const Rational<T> operator*(const Rational<T>& lhs,
                            const Rational<T>& rhs)
{ ... }
```
Now if we have this
```cpp
Rational<int> oneHalf(1, 2);          // this example is from Item 24,
                                      // except Rational is now a template

Rational<int> result = oneHalf * 2;   // error! won't compile
```
The template suggested `lhs` and `rhs` are of the same type, so we can't multiply a `Rational<int>` with a `int`.

Having deduced `lhs` and `T = int`, you might want compilers to use implicit conversion and convert `2` to `Rational<int>` and succeed, but **implicit type conversions are never considered during template argument deduction**.
Such conversions are used during function calls, but before you can call a function, you have to know which functions exist.

This instead will work:
```cpp
template<typename T>
class Rational {
public:
  ...

friend                                              // declare operator*
  const Rational operator*(const Rational& lhs,     // function (see
                           const Rational& rhs);    // below for details)
};

template<typename T>                                // define operator*
const Rational<T> operator*(const Rational<T>& lhs, // functions
                            const Rational<T>& rhs)
{ ... }
```
We relieve compilers the work of having to do type deduction, by leveraging the fact that a `friend` declaration in a template class can refer to a specific function. (Class templates don't depend on template argument deduction (that process only applies to function templates)), so `T` is always known at the time the class `Rational<T>` is instantiated.

So what happens here is `oneHalf` will cause `Rational<int>` to be instantiated, when that happens, the `friend` function declaration happens, and as a declared function, compilers no longer need to do type deduction on it, just try to generate code to call it and apply implicit conversion when needed, which in this case will be able to turn `int` into `Rational<int>`.

Note the syntax for declaring the `friend` function, without `<T>` just saves typing, the following is the same.
```cpp
template<typename T>
class Rational {
public:
  ...
friend
   const Rational<T> operator*(const Rational<T>& lhs,
                               const Rational<T>& rhs);
  ...
};
```

Now this will compile but will not link, since compiler knows we want to call `operator*(Rational<int>, Rational<int>)`, that function is declared in `Rational` but not defined there.
We want the template function outside to provide the definition, but things don't work that way: if we declare this `friend` function, we are also responsible for defining it.

The simplest approach is this:
```cpp
template<typename T>
class Rational {
public:
  ...

friend const Rational operator*(const Rational& lhs, const Rational& rhs)
{
  return Rational(lhs.numerator() * rhs.numerator(),       // same impl
                  lhs.denominator() * rhs.denominator());  // as in
}                                                          // Item 24
};
```
An interesting observation about this technique is that the use of friendship has nothing to do with needing to access non-public parts of the class.

In order to make type conversions possible on all arguments, we need a non-member function (Item 24 still applies);
And in order to have the proper function automatically instantiated, we need to declare the function inside the class.
The only way to declare a non-member function inside a class is to make it a friend. So that's what we do.

Or you can do this instead with a helper function call, say, you want to avoid the `inline`.
```cpp
template<typename T> class Rational;                 // declare
                                                     // Rational
                                                     // template
template<typename T>                                    // declare
const Rational<T> doMultiply(const Rational<T>& lhs,    // helper
                             const Rational<T>& rhs);   // template
template<typename T>
class Rational {
public:
  ...

friend
  const Rational<T> operator*(const Rational<T>& lhs,
                              const Rational<T>& rhs)   // Have friend
  { return doMultiply(lhs, rhs); }                      // call helper
  ...
};

// doMultiply impl
template<typename T>                                      // define
const Rational<T> doMultiply(const Rational<T>& lhs,      // helper
                             const Rational<T>& rhs)      // template in
{                                                         // header file,
  return Rational<T>(lhs.numerator() * rhs.numerator(),   // if necessary
                     lhs.denominator() * rhs.denominator());
}
```
`doMultiply` does not need to support mixed-mode multiplication, but it doesn't need to.
It will only be called by `operator*`, and `operator*` does support mixed mode.

In essence, `operator*` makes sure implicit conversion happens, and when both become the same `Rational<T>`, `doMultiply` does the action.

**Takeaways**
* When writing a class template that offers functions related to the template that support implicit type conversions on all parameters, define those functions as friends inside the class template

### Uses traits classes for information about types

STL has templates for containers, algorithms, iterators, etc, but also utilities.
Among its utilities templates there is `advance`.
```cpp
template<typename IterT, typename DistT>       // move iter d units
void advance(IterT& iter, DistT d);            // forward; if d < 0,
                                               // move iter backward
```
Conceptually `advance` is `iter += d`, but only random access iterators support such.
Less powerful iterators need `++` `--` iteratively `d` times.

There are five categories of STL iterators:
* **Input iterators** can only move forward one step at a time, read what they point to only once (like a read pointer to an input file, e.g. `istream_iterators`)
* **Output iterators** can only move forward one step at a time, write what they point to only once (e.g. `ostream_iterators`)
* **Forward iterator** is more powerful. They can move forward, read or write what they point to multiple times. The STL offers no singly linked list, but if one were offered it would come with a forward iterator.
* **Bidirectional iterators** adds the ability to move backward. STL `list` iterator is in this category, as for iterators for `set`, `multiset`, `map` and `multimap`
* **Random access iterator** adds to bidirectional iterator the ability to perform iterator arithmetic, to jump forward or backward a distance in constant time. Iterators for `vector`, `dequeu` and `string` are random access iterators.

For each of the five categories, C++ has a tag struct that serves to identify it
```cpp
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag: public input_iterator_tag {};
struct bidirectional_iterator_tag: public forward_iterator_tag {};
struct random_access_iterator_tag: public bidirectional_iterator_tag {};
```

Now back to `advance`, what we really want to do is this
```cpp
template<typename IterT, typename DistT>
void advance(IterT& iter, DistT d) {
  if (iter is a random access iterator) {
     iter += d;                                      // use iterator arithmetic
  }                                                  // for random access iters
  else {
    if (d >= 0) { while (d--) ++iter; }              // use iterative calls to
    else { while (d++) --iter; }                     // ++ or -- for other
  }                                                  // iterator categories
}
```
We need this information about `iter` during compilation, which `traits` let you do.

Traits is not a keyword, and needs to work for built-in types as well.
The standard technique is to put it into a template and one or more specializations of the template.
Like this
```cpp
template<typename IterT>          // template for information about
struct iterator_traits;           // iterator types
```
The way `iterator_traits` works is that for each type `IterT`, a `typedef` named `iterator_category` is declared in the struct `iterator_traits<IterT>`.
This `typedef` identifies the iterator category of `IterT`.

`iterator_traits` implements this in two parts.
Any user defined iterator type must contain a nested typedef named `iterator_category` that identifies the appropriate tag struct.
Like for `deque` and `list`:
```cpp
template < ... >                    // template params elided
class deque {
public:
  class iterator {
  public:
    typedef random_access_iterator_tag iterator_category;
    ...
  }:
  ...
};

template < ... >
class list {
public:
  class iterator {
  public:
    typedef bidirectional_iterator_tag iterator_category;
    ...
  }:
  ...
};

// iterator_traits just parrots back the iterator class's nested typedef

// the iterator_category for type IterT is whatever IterT says it is;
// see Item 42 for info on the use of "typedef typename"
template<typename IterT>
struct iterator_traits {
  typedef typename IterT::iterator_category iterator_category;
  ...
};
```
This works well for user defined types, but not for iterators that are pointers, since there's no such thing as a pointer with a nested `typedef`.
Thus the second part of the `iterator_traits` implementation handles iterators that are pointers, by offering a **partial template specialization** for pointer types.
```cpp
template<typename IterT>               // partial template specialization
struct iterator_traits<IterT*>         // for built-in pointer types
{
  typedef random_access_iterator_tag iterator_category;
  ...
};
```
So to design a `traits` class:
* identify some information you'd like to make available (for iterators, their category)
* choose a name to identify that information (e.g. `iterator_category`)
* provide a template and set of specializations (e.g. `iterator_traits` taht contain the information for the types you want to support)

And `advance` looks like
```cpp
template<typename IterT, typename DistT>
void advance(IterT& iter, DistT d) {
  if (typeid(typename std::iterator_traits<IterT>::iterator_category) ==
     typeid(std::random_access_iterator_tag))
  ...
}
```
But `typeid` is runtime, while at compile time we have all the information.
We need an `if...else...` for types that is evaluated during compilation, we can achieve this via overloading.
Like this
```cpp
template<typename IterT, typename DistT>              // use this impl for
void doAdvance(IterT& iter, DistT d,                  // random access
               std::random_access_iterator_tag)       // iterators
{
  iter += d;
}

template<typename IterT, typename DistT>              // use this impl for
void doAdvance(IterT& iter, DistT d,                  // bidirectional
               std::bidirectional_iterator_tag)       // iterators
{
  if (d >= 0) { while (d--) ++iter; }
  else { while (d++) --iter;        }
}

template<typename IterT, typename DistT>              // use this impl for
void doAdvance(IterT& iter, DistT d,                  // input iterators
               std::input_iterator_tag)
{
  if (d < 0 ) {
     throw std::out_of_range("Negative distance");    // see below
  }
  while (d--) ++iter;
}
```
Because `forward_iterator_tag` inherits from `input_iterator_tag`, the version taking `input_iterator_tag` will also work for `forward_iterators`.
And code for `advance` looks like
```cpp
template<typename IterT, typename DistT>
void advance(IterT& iter, DistT d) {
  doAdvance(                                              // call the version
    iter, d,                                              // of doAdvance
    typename                                              // that is
      std::iterator_traits<IterT>::iterator_category()    // appropriate for
  );                                                      // iter's iterator
}                                                         // category
```
So how to use a traits class:
* create a set of overloaded worker functions that differ in a traits parameter. Implement each function in accord with the traits information passed
* create a master or function template that calls the workers, passing information provided by a traits class

Traits are widely used in standard library.
There's `iterator_traits` which offers `iterator_category`, `value_type`, etc.
There's also `char_traits`, `numeric_limits`.
And TR1 introduces `is_fundamental<T>`, `is_array<T>`, `is_base_of<T1, T2>`.

**Takeaways**
* Traits classes make information about types available during compilation. They're implemented using templates and template specializations
* In conjunction with overloading, traits classes make it possible to perform compile-time `if...else` tests on types

### Be aware of template meta programming

TMP is the technique that writes template-based C++ programs that execute during compilation.

TMP makes some things easy that would otherwise be hard or impossible.
They also shift the work from runtime to compile time.

Like the example with `advance` from previous item, this is a version where work is done at runtime
```cpp
template<typename IterT, typename DistT>
void advance(IterT& iter, DistT d) {
  if (typeid(typename std::iterator_traits<IterT>::iterator_category) ==
      typeid(std::random_access_iterator_tag)) {
     iter += d;                                     // use iterator arithmetic
  } else {                                          // for random access iters
    if (d >= 0) { while (d--) ++iter; }             // use iterative calls to
    else { while (d++) --iter; }                    // ++ or -- for other
  }                                                 // iterator categories
}
```

Item 47 shows how `traits` can be more efficient, in fact, the `traits` approach is TMP.

If `advance` looks like the above, consider this code
```cpp
std::list<int>::iterator iter;
advance(iter, 10);                          // move iter 10 elements forward;
                                            // won't compile with above impl.
```
This won't compile, as `iter += d` doesn't work on `list`'s bidirectional iterator.
We know the line `iter += d` will never be executed, but compiler doesn't know that, as `typeid` is a runtime check.
The `traits` TMP approach doesn't have the same problem.

Boost's `mpl` offers a higher level TMP syntax, something that looks very different from ordinary C++.

TMP uses recursive template instantiations to realize recursion (loops).

E.g. this TMP factorial
```cpp
template<unsigned n>                 // general case: the value of
struct Factorial {                   // Factorial<n> is n times the value
                                     // of Factorial<n-1>
  enum { value = n * Factorial<n-1>::value };
};

template<>                           // special case: the value of
struct Factorial<0> {                // Factorial<0> is 1
  enum { value = 1 };

};
```
You get the factorial of `n` by referring to `Factorial<n>::value`.
This uses enum hack described in item 2.

Why is TMP worth knowing about, some examples:
* ensuring dimensional unit correctness (like, a variable representing mass cannot be assigned to a variable representing velocity, but can be divided by such)
* optimizing matrix operations. This code
```cpp
typedef SquareMatrix<double, 10000> BigMatrix;
BigMatrix m1, m2, m3, m4, m5;               // create matrices and
...                                         // give them values
BigMatrix result = m1 * m2 * m3 * m4 * m5;  // compute their product
```
The normal way calls for four temporary matrices and independent for loops, using **TMP expression templates** it's possible to avoid temporaries and merge the loops.
* Generating custom design pattern implementations. **policy-based design**, **generative programming**

**Takeaways**
* Template metaprogramming can shift work from runtime to compile-time, thus enabling earlier error detection and higher runtime performance
* TMP can be used to generate custom code based on combinations of policy choices, and it can also be used to avoid generating code inappropriate for particular types
