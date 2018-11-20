# Moving to Modern C++

### Item 7: distinguish between () and {} when creating objects

Different ways of initializing

```cpp
int x(0);             // initializer is in parentheses
int y = 0;            // initializer follows "="
int z{ 0 };           // initializer is in braces
int z = { 0 };        // initializer uses "=" and braces, generally the same as braces
```

First differentiate assignment operator and copy constructor

```cpp
Widget w1;            // call default constructor
Widget w2 = w1;       // not an assignment; calls copy ctor
w1 = w2;              // an assignment; calls copy operator=
```

C++11 introduces the concept of **uniform initialization** using braces.

```cpp
std::vector<int> v{ 1, 3, 5 }; // v's initial content is 1, 3, 5, not possible in c++03

class Widget {
  …

private:
  int x{ 0 };         // fine, x's default value is 0
  int y = 0;          // also fine
  int z(0);           // error!
};

// Uncopiable objects can use {}, (), but not = to initialize
std::atomic<int> ai1{ 0 };     // fine
std::atomic<int> ai2(0);       // fine
std::atomic<int> ai3 = 0;      // error!

// braced initialization prohibits implicit narrowing conversions among built-in types
double x, y, z;
int sum1{ x + y + z };       // error! sum of doubles may
                             // not be expressible as int
int sum2(x + y + z);         // okay (value of expression
                             // truncated to an int)
int sum3 = x + y + z;        // ditto

// C++ parse says anything that can be interpretted as a declaration must be interpretted as one, thus
Widget w1(10);     // call Widget ctor with argument 10
Widget w2();       // most vexing parse! declares a function
                   // named w2 that returns a Widget!
Widget w3{};       // calls Widget ctor with no args
```

Why not always use braced initialization then?

There can be unexpected behaviors due to the tangled relationship among initializers, `std::intializer_lists`, and constructor overload resolution.

E.g. in [item 2](it1-4-deducing-types.md#understand-auto-type-deduction), deduced type for auto using braced initialization is `std::initializer_list`.
(The more you like auto, the less you may like braced initialization)

```cpp
// Braced initialization always prefers a constructor overload that takes in
// std::initialization_list, and what would normally be copy and move
// construction can be hijacked by std::initialization_list ctors

class Widget {
public:
  Widget(int i, bool b);
  Widget(int i, double d);

  Widget(std::initializer_list<long double> il);

  operator float() const;                          // convert
};

Widget w1(10, true);     // uses parens and, as before,
                         // calls first ctor

Widget w2{10, true};     // uses braces, but now calls
                         // std::initializer_list ctor
                         // (10 and true convert to long double)

Widget w3(10, 5.0);      // uses parens and, as before,
                         // calls second ctor

Widget w4{10, 5.0};      // uses braces, but now calls
                         // std::initializer_list ctor
                         // (10 and 5.0 convert to long double)

Widget w5(w4);               // uses parens, calls copy ctor

Widget w6{w4};               // uses braces, calls
                             // std::initializer_list ctor
                             // (w4 converts to float, and float
                             // converts to long double)

Widget w7(std::move(w4));    // uses parens, calls move ctor

Widget w8{std::move(w4)};    // uses braces, calls
                             // std::initializer_list ctor
                             // (for same reason as w6)

// Compiler will even block calling other matched ctors, if
// std::initializer_list ctor exists + braced initialization,
// but the braced initialization requires narrowing
// conversions. (no conversions is fine)

// And if you have a default ctor and a initializer_list ctor
Widget w1;            // calls default ctor
Widget w2{};          // also calls default ctor
Widget w3();          // most vexing parse! declares a function!
Widget w4({});        // calls std::initializer_list ctor
                      // with empty list
```

Why does this matter?
Consider `std::vector`, it has a (length, value) ctor and a `std::initializer_list` ctor.
And the difference could be such.
```cpp
std::vector<int> v1(10, 20);  // use non-std::initializer_list
                              // ctor: create 10-element
                              // std::vector, all elements have
                              // value of 20

std::vector<int> v2{10, 20};  // use std::initializer_list ctor:
                              // create 2-element std::vector,
                              // element values are 10 and 20
```
That means as a class designer, don't do what `std::vector` does.

It's best to design your constructors so that the overload called isn't affected by whether clients use parentheses or braces.

And if you need to add a `std::initializer_list` ctor, do so with great deliberation.

The second lesson is that as a class client, you must choose carefully between parentheses and braces when creating objects.

**Takeaway**
* Braced initialization is the most widely usable initialization syntax, it prevents narrowing conversions, and it’s immune to C++’s most vexing parse.
* During constructor overload resolution, braced initializers are matched to `std::initializer_list` parameters if at all possible, even if other constructors offer seemingly better matches.
* An example of where the choice between parentheses and braces can make a significant difference is creating a `std::vector<numeric type>` with two arguments.
* Choosing between parentheses and braces for object creation inside templates can be challenging. One could do braces only when necessary; or instead, always do `{}` but understand when semantically `()` is desirable


### Item 8: prefer nullptr to 0 and NULL

Neither `0` or `NULL` has a pointer type.
In C++98, the primary implication of this was overloads on pointer and integral types could be surprising.

```cpp
void f(int);        // three overloads of f
void f(bool);
void f(void*);

f(0);               // calls f(int), not f(void*)

f(NULL);            // might not compile (if NULL is 0L, since
                    // the conversion from long to int, to bool
                    // is equally good), but typically calls
                    // f(int) if NULL is 0. Never calls f(void*)

f(nullptr);         // calls f(void*) overload
```

`nullptr` does not have an integral type.
It does not suffer from overload resolution surprises that `0` and `NULL` are susceptible to.
It doesn't have a pointer type either, but you can think of it as a pointer of all types.
Its actual type is `std::nullptr_t`, which implicitly converts to all pointer types.

`nullptr` improves code clarity. Consider
```cpp
auto result = findRecord( /* arguments */ );
if (result == nullptr) { ... }

// vs
auto result = findRecord( /* arguments */ );
if (result == 0) { ... }
```

`nullptr` shines even more in template specialization.

**Takeaways**
* Prefer `nullptr` to `0` and `NULL`.
* Avoid overloading on integral and pointer types.

### Item 9: prefer alias declarations to `typedefs`

Alias is easier to swallow when dealing with types involving function pointers
```cpp
// FP is a synonym for a pointer to a function taking an int and
// a const std::string& and returning nothing
typedef void (*FP)(int, const std::string&);      // typedef

// same meaning as above
using FP = void (*)(int, const std::string&);     // alias
                                                  // declaration
```

A more compelling reason to use alias is that they work with having templates inside

```cpp
// alias (C++11)
template<typename T>                           // MyAllocList<T>
using MyAllocList = std::list<T, MyAlloc<T>>;  // is synonym for
                                               // std::list<T,
                                               //   MyAlloc<T>>

MyAllocList<Widget> lw;                        // client code

// And to use MyAllocList within another template

template<typename T>
class Widget {
private:
  MyAllocList<T> list;                         // compared with below
                                               // no "typename",
  …                                            // no "::type"
};

// typedef (C++03)
template<typename T>                     // MyAllocList<T>::type
struct MyAllocList {                     // is synonym for
  typedef std::list<T, MyAlloc<T>> type; // std::list<T,
};                                       //   MyAlloc<T>>

MyAllocList<Widget>::type lw;            // client code

// It gets worse. If you use typedef inside a template for the purpose
// of creating a linked list holding objects of a type specified by a
// template parameter, you have to precede the typedef name with typename,
// because MyAllocList<T>::type is now a dependent type (nested dependent name).
//
// Compiler doesn't know for sure MyAllocList<T>::type is a type. There might be
// a specialization of MyAllocList<T> somewhere that has ::type not as a type,
// but as a data member. Compiler doesn't know for sure.)

template<typename T>
class Widget {                         // Widget<T> contains
private:                               // a MyAllocList<T>
  typename MyAllocList<T>::type list;  // as a data member
  …
};
```

C++11 has type traits to perform type transformation, in C++11, it's implemented with enclosing structs (C++03 typedef), and in C++14 it's done with alias. So we have

```cpp
std::remove_const<T>::type           // C++11: const T → T
std::remove_const_t<T>               // C++14 equivalent

std::remove_reference<T>::type       // C++11: T&/T&& → T
std::remove_reference_t<T>           // C++14 equivalent

std::add_lvalue_reference<T>::type   // C++11: T → T&
std::add_lvalue_reference_t<T>       // C++14 equivalent
```

**Takeaways**
* `typedef`s don’t support templatization, but alias declarations do.
* Alias templates avoid the `::type` suffix and, in templates, the `typename` prefix often required to refer to typedefs.
* C++14 offers alias templates for all the C++11 type traits transformations.

### Item 10: prefer scoped enums to unscoped enums

As a general rule, declaring a name inside curly braces limits the visibility of that name to the scope defined by the braces.
Not so for the enumerators declared in C++98-style enums.

The names of such enumerators belong to the scope containing the enum (definition is leaked into the enclosing scope, thus unscoped enums), and that means that nothing else in that scope may have the same name:

```cpp
// C++03
enum Color { black, white, red };   // black, white, red are
                                    // in same scope as Color

auto white = false;                 // error! white already
                                    // declared in this scope
// C++11
enum class Color { black, white, red };  // black, white, red
                                         // are scoped to Color

auto white = false;              // fine, no other
                                 // "white" in scope

Color c = white;                 // error! no enumerator named
                                 // "white" is in this scope

Color c = Color::white;          // fine

auto c = Color::white;           // also fine (and in accord
                                 // with Item 5's advice)
```

Scoped enums are declared via `enum class`.
They are referred to as enum classes as well.

Enum classes
* Reduce namespace pollution
* Are strongly typed (no implicit conversion to other types, while unscoped enum implicitly convert to integral types)

Enum classes can be forward declared (by default the underlying type for scoped enums in `int`, so the compiler knows the size of a forward declared enum.
You can override the default underlying type).

Unscoped enum can be forward declared only if the underlying type is specified.

```cpp
enum class Status;                 // underlying type is int,

enum class Status: std::uint32_t;  // underlying type for
                                   // Status is std::uint32_t
                                   // (from <cstdint>)


enum Color: std::uint8_t;       // fwd decl for unscoped enum;
                                // underlying type is
                                // std::uint8_t

// Underlying type specifications can also go on an enum’s definition:
enum class Status: std::uint32_t { good = 0,
                                   failed = 1,
                                   incomplete = 100,
                                   corrupt = 200,
                                   audited = 500,
                                   indeterminate = 0xFFFFFFFF
                                 };
```

There is a case where unscoped enums may be useful due to its implicit conversion to integral types, say you have the following.

```cpp
using UserInfo =                 // type alias; see Item 9
  std::tuple<std::string,        // name
             std::string,        // email
             std::size_t> ;      // reputation

// elsewhere you see this
UserInfo uInfo;                  // object of tuple type
…
auto val = std::get<1>(uInfo);   // get value of field 1

// You probably don't want to remember what fields 1, 2, 3 are,
// so you could have this instead
enum UserInfoFields { uiName, uiEmail, uiReputation };

UserInfo uInfo;                        // as before
…

auto val = std::get<uiEmail>(uInfo);   // ah, get value of
                                       // email field

// And the corresponding code with scoped enums is substantially
// more verbose:

enum class UserInfoFields { uiName, uiEmail, uiReputation };

UserInfo uInfo;                        // as before
…

auto val =
  std::get<static_cast<std::size_t>(UserInfoFields::uiEmail)>
    (uInfo);

// You could get rid of some of this verbosity using a function, but
// that function has to be evaluated at compile time, since std::get
// is a template.
template<typename E>                               // C++14
constexpr auto toUType(E enumerator) noexcept
{
  return static_cast<std::underlying_type_t<E>>(enumerator);
}

// And you'd be able to do
auto val = std::get<toUType(UserInfoFields::uiEmail)>(uInfo);
```

**Takeaways**
* C++98-style enums are now known as unscoped enums.
* Enumerators of scoped enums are visible only within the enum. They convert to other types only with a cast.
* Both scoped and unscoped enums support specification of the underlying type. The default underlying type for scoped enums is `int`. Unscoped enums have no default underlying type (implementation-dependent integral type that can represent all enumerator values).
* Scoped enums may always be forward-declared. Unscoped enums may be forward-declared only if their declaration specifies an underlying type.

### Item 11: prefer deleted functions to private undefined ones

You often want to suppress the special member functions that the compiler generates for you, like copycon and assignment operator.
In C++03 you do that with private undefined copycon and assignment opr.
In C++11 you do that with declaration with `= delete`.

```cpp
// C++03
template <class charT, class traits = char_traits<charT> >
class basic_ios : public ios_base {
public:
  …

private:
  basic_ios(const basic_ios& );            // not defined
  basic_ios& operator=(const basic_ios&);  // not defined
};

// C++11
template <class charT, class traits = char_traits<charT> >
class basic_ios : public ios_base {
public:
  …
  basic_ios(const basic_ios& ) = delete;
  basic_ios& operator=(const basic_ios&) = delete;
  // these are public since compiler checks deleted status before accessibility
  …
};
```
The advantages of `= delete`
* `= delete` will result in better error messages: always at compile time (as opposed to friends / members seeing undefined symbols)
* any functions can be deleted, while only member functions can be made private. You can use this to get rid of unwanted implicit conversions, or unwanted template instantiation. E.g.
```cpp
bool isLucky(int number);
if (isLucky('a')) …            // is 'a' a lucky number?

if (isLucky(true)) …           // is "true"?

if (isLucky(3.5)) …            // should we truncate to 3
                               // before checking for luckiness?

// Such implicit conversions to int are undesirable, you could do
bool isLucky(int number);            // original function

bool isLucky(char) = delete;         // reject chars

bool isLucky(bool) = delete;         // reject bools

bool isLucky(double) = delete;       // reject doubles and
                                     // floats
// and you'll have
if (isLucky('a')) …           // error! call to deleted function

if (isLucky(true)) …          // error!

if (isLucky(3.5f)) …          // error!

// And for unwanted template instantiations
template<typename T>
void processPointer(T* ptr);

template<>
void processPointer<void>(void*) = delete;

template<>
void processPointer<char>(char*) = delete;

template<>
void processPointer<const void>(const void*) = delete;

template<>
void processPointer<const char>(const char*) = delete;

// Similarly, for template class member functions

class Widget {
public:
  …
  template<typename T>
  void processPointer(T* ptr)
  { … }
  …

};

template<>                                          // still
void Widget::processPointer<void>(void*) = delete;  // public,
                                                    // but
                                                    // deleted
```

**Takeaways**
* Prefer deleted functions to private undefined ones
* Any function may be deleted, including non-member functions and template instantiations

### Item 12: declare overriding functions `override`

`override` has nothing to do with overload.
`override` made it possible to invoke a derived class function through a base class interface.

For `override` to occur
* Base class function must be `virtual`
* Base and derived function names must be identical (except in dtor)
* Parameter types must be identical
* constness of the base and derived functions must be identical
* return types and exception sepcifications must be compatible
* (C++11) function's reference qualifier must be identical

Reference qualifiers can make a member function available to lvalues or rvalues only.
```cpp
class Widget {
public:
  …
  void doWork() &;       // this version of doWork applies
                         // only when *this is an lvalue

  void doWork() &&;      // this version of doWork applies
};                       // only when *this is an rvalue

…

Widget makeWidget();     // factory function (returns rvalue)

Widget w;                // normal object (an lvalue)

…

w.doWork();              // calls Widget::doWork for lvalues
                         // (i.e., Widget::doWork &)

makeWidget().doWork();   // calls Widget::doWork for rvalues
                         // (i.e., Widget::doWork &&)
```
How is this useful? Consider this code
```cpp
class Widget {
public:
  using DataType = std::vector<double>;      // see Item 9 for
  …                                          // info on "using"

  DataType& data() { return values; }
  …

private:
  DataType values;
};

Widget w;
auto vals1 = w.data();               // copy w.values into vals1

// Now suppose we have a factory function that creates Widgets,
Widget makeWidget();

auto vals2 = makeWidget().data();    // copy values inside the
                                     // Widget into vals2

// However in this case we don't actually need to copy since
// the original copy in makeWidget is a temporary. Move is
// preferable. Compiler may be able to optimize this, but
// don't depend on it. We could have this instead, move if
// data() is called on an rvalue, copy if it's called on an
// lvalue

class Widget {
public:
  using DataType = std::vector<double>;
  …

  DataType& data() &                // for lvalue Widgets, 
  { return values; }                // return lvalue

  DataType&& data() &&              // for rvalue Widgets,
  { return std::move(values); }     // return rvalue
  …

private:
  DataType values;
};
```

Back to the matter at hand, small mistakes can cause you to think something overrides while in fact it doesn't. E.g.
```cpp
class Base {
public:
  virtual void mf1() const;
  virtual void mf2(int x);
  virtual void mf3() &;
  void mf4() const;
};

class Derived: public Base {
public:
  virtual void mf1();
  virtual void mf2(unsigned int x);
  virtual void mf3() &&;
  virtual void mf4() const;
};
```
Compilers don't have to emit warnings in this case.

Because declaring `override` is important to get right and easy to get wrong, C++11 introduces declaring a function `override`.
In which case you are asking the compiler to help check something is indeed overridden.
```cpp
class Derived: public Base {
public:
  virtual void mf1() override;
  virtual void mf2(unsigned int x) override;
  virtual void mf3() && override;
  virtual void mf4() const override;
};
```

It also helps you gauge the ramifications if you are contemplating changing the signature of a virtual function in a base class. You can see how many derived classes fails to compile.

`override` and `final` are contextual keywords: they are reserved only in a context.
In `override`'s case, at the end of a member function declaration.

**Takeaways**
* Declare overriding functions override
* Member function reference qualifiers make it possible to treat lvalue and rvalue objects (`*this`) differently

### Item 13: prefer `const_iterator`s to `iterator`s

This is in line with use `const` whenever possible.
Problem is STL in C++98 has `const_iterator`s, but a lot of functions expect `iterator`s (e.g. `vector.insert`) as opposed to `const_iterator`s, making adopting `const_iterator`s hard. E.g.

```cpp
// To find a value replace it with another one if found, otherwise
// insert to the end. This is possible with const_iterators in C++11,
// not before.
std::vector<int> values;                           // as before
...
auto it =                                          // use cbegin
  std::find(values.cbegin(), values.cend(), 1983); // and cend

values.insert(it, 1998);

// before, you may be able to do something like this
typedef std::vector<int>::iterator IterT;             // type-
typedef std::vector<int>::const_iterator ConstIterT;  // defs

std::vector<int> values;

…

ConstIterT ci =
  std::find(static_cast<ConstIterT>(values.begin()),  // cast
            static_cast<ConstIterT>(values.end()),    // cast
            1983);

values.insert(static_cast<IterT>(ci), 1998);    // may not
                                                // compile
```

To write maximally generic library code, take into account that some containers and container-like data structures offer `begin` and `end` (plus `cbegin`, `cend`, `rbegin`, etc.) as non-member functions, rather than members.
This is the case for built-in arrays, for example, and it’s also the case for some third-party libraries with interfaces consisting only of free functions.
Maximally generic code thus uses non-member functions rather than assuming the existence of member versions.

(C++11 had non-member versions of `begin`, `end`, but forgot to add non-member versions of `cbegin`, `cend`, `rbegin`, `rend`, `crbegin`. This is corrected in C++14)

**Takeaways**
* Prefer `const_iterator`s to `iterator`s.
* In maximally generic code, prefer non-member versions of `begin`, `end`, `rbegin`, etc., over their member function counterparts.

### Item 14: declare functions `noexcept` if they won't emit exceptions

C++98 compiler offers no help in checking exception specification: programmers summarize the possible exception specifications, and update them as code changes.
C++11 allows indication of if a function may emit any exceptions.

Why noexcept:
* Failure to declare a function noexcept when you know that it won't throw an exception is poor interface specification: whether a function is `noexcept` is as important a piece of information as whether a member function is `const`.
* Compiler may generate more efficient code for `noexcept`: the difference between unwinding the stack and possibly unwinding it has a surprisingly large impact on code generation.

```cpp
RetType function(params) noexcept;     // most optimizable

RetType function(params) throw();      // less optimizable

RetType function(params);              // less optimizable
```

More motivations in practice:

Think of `vector`'s insert implementation.
In C++98, it's allocate, copy, then remove.

This has [strong exception safe guarantee](../ecpp/it26-31-implementations.md#strive-for-exception-safe-code): if an exception is thrown during copying, the state of the vector will be unchanged.
(`push_back` does not guarantee noexcept though)

In C++11 we can leverage move.
If move doens't have `noexcept`, `push_back`'s exception safety guarantee is violated: a move throwing an exception may cause the vector to be in a different state.
What about move back on exception?
Still no guarantee since move back may throw an exception itself.

Thus the `push_back` implementation in C++11 leverages move when it can (i.e. move has been declared `noexcept`).
Other STL functions do that, too: "move if you can, copy if you must".

`swap` functions comprise another case where noexcept is desirable.
Heavily used in STL and assignment operator.
Often times a whether `swap` is defined as `noexcept` depends on user-defined types are `noexcept`: conditionally noexcept e.g.

```cpp
template <class T, size_t N>
void swap(T (&a)[N],                                    // see
          T (&b)[N]) noexcept(noexcept(swap(*a, *b)));  // below

template <class T1, class T2>
struct pair {
  …
  void swap(pair& p) noexcept(noexcept(swap(first, p.first)) &&
                              noexcept(swap(second, p.second)));
  …
};
```

Optimization is important, but correctness is more important.

Declare something `noexcept` only if you are willing to commit to this function being noexcept in the future.
This is part of the interface (in this case, the agreement between you and your client code), and you risk breaking client code if you change your mind.

The fact is most functions are **exception neutral**: they don't throw, but what they call might.
If so, these exception neutral functions are rightfully defined as not `noexcept`.

If some function (e.g. `swap`) has natural `noexcept` impls, it's worth implementing them that way and declaring `noexcept`.
But if not and you tweak the impl (e.g. an underlying call might throw and you catch all possible ones and return different values) such that it's `noexcept`, it'd be putting the cart before the horse.

By default, some functions are implicitly `noexcept` (e.g. dtors).

Some impls differentiate functions by wide contract and narrow contract.

**Wide contract** means this function has no precondition (unaware of program states) and it imposes no constraints on the arguments it's given. They never exhibit UB.

**Narrow contract** means otherwise: if a precondition is violated, results are undefined.

Typically we declare `noexcept` on wide contract functions, and situation is trickier with narrow contract functions.

Compilers offer no help in detecting `noexcept`.
The following compiles without warning. Reasoning being the called function might be in C, might be in C++98 style, where `noexcept` doesn't exist.

```cpp
void a();
void b() noexcept {
    a();
    ...
}
```

**Takeaways**
* `noexcept` is part of a function’s interface, and that means that callers may depend on it.
* `noexcept` functions are more optimizable than non-`noexcept` functions.
* `noexcept` is particularly valuable for the move operations, `swap`, memory deallocation functions, and dtors.
* Most functions are exception-neutral rather than `noexcept`.

### Item 15: use `constexpr` whenever possible

Conceptually, `constexpr` indicates a value's not only constant but also it's known during compilation.

`constexpr` objects and functions have different meanings.
`constexpr` objects are `const` and known at compile time.
Values that are known during compile time may be placed in read-only memory.

And of broader applicability is that integral values that are constant and known during compilation can be used in contexts where C++ requires an integral `constexpr`: e.g. array sizes, integral template arguments, enumerator values, alignment specifiers, etc. E.g.

```cpp
int sz;                             // non-constexpr variable

…

constexpr auto arraySize1 = sz;     // error! sz's value not
                                    // known at compilation

std::array<int, sz> data1;          // error! same problem

constexpr auto arraySize2 = 10;     // fine, 10 is a
                                    // compile-time constant

std::array<int, arraySize2> data2;  // fine, arraySize2
                                    // is constexpr

// Note const doesn't offer the same guarantee: const needs not be
// initialized with values known during compilation
// All constexpr objects are const, not all consts are constexpr
const auto arraySize = sz;          // fine, arraySize is
                                    // const copy of sz

std::array<int, arraySize> data;    // error! arraySize's value
                                    // not known at compilation
```

`constexpr` functions produce compile-time constants (computed during compilation) when they are called with compile-time constants.
If they are called with values not known until runtime, they produce runtime values.

In C++11, `constexpr` may contain no more than a single executable statement: a return. (But you can do recursions to get loops and ternary expr to get `if`s).

In C++14, `constexpr` are limited to taking and returning **literal types**, meaning types that can have values determined during compilation.
In C++11, all built-in types except `void` are literal types, and user defined types may be, too, if they have their ctors and some other member functions constexpr. E.g.

```cpp
class Point {
public:
  constexpr Point(double xVal = 0, double yVal = 0) noexcept
  : x(xVal), y(yVal)
  {}

  constexpr double xValue() const noexcept { return x; }
  constexpr double yValue() const noexcept { return y; }

  // these aren't constexpr since in C++11 constexpr are implicitly
  // const, and they return void which isn't a literal type in C++11.
  void setX(double newX) noexcept { x = newX; }
  void setY(double newY) noexcept { y = newY; }

  // in C++14 both these constraints are lifted. You can do.
  // constexpr void setX(double newX) noexcept     // C++14
  // { x = newX; }

  // constexpr void setY(double newY) noexcept     // C++14
  // { y = newY; }
private:
  double x, y;
};

// And it's fine to do the following (evaluated at compile time)
constexpr Point p1(9.4, 27.7);      // fine, "runs" constexpr
                                    // ctor during compilation

constexpr Point p2(28.8, 5.3);      // also fine

constexpr
Point midpoint(const Point& p1, const Point& p2) noexcept
{
  return { (p1.xValue() + p2.xValue()) / 2,    // call constexpr
           (p1.yValue() + p2.yValue()) / 2 };  // member funcs
}

constexpr auto mid = midpoint(p1, p2);     // init constexpr
                                           // object w/result of
                                           // constexpr function

// and with C++14 (setter constexpr) you can do the following
// return reflection of p with respect to the origin (C++14)
constexpr Point reflection(const Point& p) noexcept
{
  Point result;                       // create non-const Point

  result.setX(-p.xValue());           // set its x and y values
  result.setY(-p.yValue());

  return result;                      // return copy of it
}
```

This blurs the line of computation at runtime with at compile time.
The more code is moved to compile time, the faster your program at runtime.
Conversely the slower to compile.

Use `constexpr` whenever possible: both `constexpr` functions and objects can be employed in a wider range of contexts than non-`constexpr` objects and functions.

Keep in mind that `constexpr` is part of the interface: use it if only you are able to commit to it.
If you later decide to remove it (like adding debug IO since they are generally not permitted), you may break an arbitrary amount of client code.

**Takeaways**
* `constexpr` objects are const and are initialized with values known during compilation.
* `constexpr` functions can produce compile-time results when called with arguments whose values are known during compilation.
* `constexpr` objects and functions may be used in a wider range of contexts than non-`constexpr` objects and functions.
* `constexpr` is part of an object’s or function’s interface.

### Item 16: make `const` member functions thread safe

If a member function is made `const`, conceptually it should be safe for multiple threads to call the same method at the same time on the same object.

However, consider the case where a `const` member function modifies a `mutable` member variable (say, `getRoot` of a `Polynomial` class modifies the `rootCache` and `isRootCacheValid`, which are declared `mutable`): a `const` member function is no longer threadsafe.

One could add a `mutex` to the `getRoot` operation. Worth noting that `std::mutex` cannot be copied or moved, by doing so `Polynomial` class loses the ability to be copied or moved. `std::atomic` might be a cheaper solution if all you want is a counter, though know that `std::atomic` are also uncopiable and unmovable.

If you require two or more memory locations to be synchronized (e.g. a `bool isValid` and an `int value`), then `std::atomic` is typically not enough.
If you only require one, they typically are.

If your code is designed for a single threaded environment then this is not a concern.
However such environments are becoming rarer.
The safe bet is that `const` member functions will be subject to concurrent execution, and that's why you should ensure your `const` member functions are threadsafe.

**Takeaways**
* Make `const` member functions threadsafe unless you're certain they'll never be used in a concurrent context.
* Use of `std::atomic` variables may offer better performance than a `mutex`, but they’re suited for manipulation of only a single variable or memory location.

### Understand special member function generation

Special member functions are those that the compiler will generate on its own.
In C++98 we have four. Default ctor, dtor, copycon, (copy) assignment opr.
They are generated only if they are needed: if you declared ctor with param, the default one won't be generated.
They are implicitly public and inline.
They are not virtual by default, except in a derived class whose parent class's dtor is virtual.

In C++11, two more special member functions are generated, move ctor and move assignment opr.
```cpp
class Widget {
public:
  …
  Widget(Widget&& rhs);              // move constructor

  Widget& operator=(Widget&& rhs);   // move assignment operator
  …
};
```
Both of them perform memberwise move on the non-static members of the class. The move ctor / assignment opr also moves its base class parts (if any).
(Use move on data member / base class that supports it, copy otherwise)

Copycon and copy assignment opr are independent: declare only one and compiler will generate the other one for you.
Movecon and move assignment opr are not: declaring a movecon will cause move assignment opr to not be generated as well, vice versa. Compiler's rationale is that if you need customized movecon, you will want custom move assignment opr as well.

Move operations won't be defined for a class with custom copycon, custom copy assignment opr, or custom dtor: if you need special copy or special dtor, you'll need special move, too.
This goes in the other direction, too. Declaring a movecon or move assignment opr causes compilers to disable copy operations.

To summarize, two moves are generated (when needed) for classes that don't have custom
* copycon or copy assignment
* dtor
* movecon or move assignment

The rule of three: if you have one of custom dtor, copycon, or copy assignment opr, you should have the other two, too.
All standard library classes that manage memory has big three defined.
Default copycon / copy assignment generation is deprecated in C++11 if custom copy assignment / copycon / dtor is present.

C++11 adds "= default" to let you specify the default memberwise approach is desired.
This is often useful in a base class where you need to declare the dtor virtual.
In which case if the default dtor is desirable and you still want the compiler generated moves, you could do the following

```cpp
class Base {
public:
  virtual ~Base() = default;                // make dtor virtual

  Base(Base&&) = default;                   // support moving
  Base& operator=(Base&&) = default;

  Base(const Base&) = default;              // support copying
  Base& operator=(const Base&) = default;

  …

};
```

In fact, it might be a good idea to specify "= default" anyway to clearly state your intention.
Without "= default", consider this case where you decided to add a dtor, the impact is profound as moves are silently deleted. (say you have a std::map in this class, previously it can be moved now it has to be copied, this is orders of magnitude slower.)

Default ctor is the same in C++11 as C++98.
Generated dtor is roughly the same, except it's noexcept by default.

If you have template copycon / copy assignment opr, like
```cpp
class Widget {
  …
  template<typename T>                // construct Widget
  Widget(const T& rhs);               // from anything

  template<typename T>                // assign Widget
  Widget& operator=(const T& rhs);    // from anything
  …
};
```

Compiler still generates the defaults (copy, move, etc) for you.

**Takeaway**
* The special member functions are those compilers may generate on their own: default constructor, destructor, copy operations, and move operations.
* Move operations are generated only for classes lacking explicitly declared move operations, copy operations, and a destructor.
* The copy constructor is generated only for classes lacking an explicitly declared copy constructor, and it’s deleted if a move operation is declared. The copy assignment operator is generated only for classes lacking an explicitly declared copy assignment operator, and it’s deleted if a move operation is declared. Generation of the copy operations in classes with an explicitly declared copy operation or destructor is deprecated.
* Member function templates never suppress generation of special member functions.

