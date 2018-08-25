# Chap 1 Deducing Types

### Understand type deduction

```cpp
template<typename T>
void f(T& param);       // param is a reference (same case goes for param is a pointer)

int x = 27;             // x is an int
const int cx = x;       // cx is a const int
const int& rx = x;      // rx is a reference to x as a const int

f(x);                   // T is int, param's type is int&

f(cx);                  // T is const int,
                        // param's type is const int&

f(rx);                  // T is const int,
                        // param's type is const int&, the refness of rx is dropped
```

```cpp
template<typename T>
void f(T&& param);       // param is now a universal reference

int x = 27;              // as before
const int cx = x;        // as before
const int& rx = x;       // as before

f(x);                    // x is lvalue, so T is int&,
                         // param's type is also int&

f(cx);                   // cx is lvalue, so T is const int&,
                         // param's type is also const int&

f(rx);                   // rx is lvalue, so T is const int&,
                         // param's type is also const int&

f(27);                   // 27 is rvalue, so T is int,
                         // param's type is therefore int&&
```

```cpp
template<typename T>
void f(T param);         // param is now passed by value

int x = 27;          // as before
const int cx = x;    // as before
const int& rx = x;   // as before

f(x);                // T's and param's types are both int

f(cx);               // T's and param's types are again both int

f(rx);               // T's and param's types are still both int
```

```cpp
// Array declaration in function params decays into pointer

void myFunc(int param[]);
void myFunc(int* param);         // same function as above
```

```cpp
template<typename T>
void f(T param);         // param is now passed by value

const char name[] = "J. P. Briggs";  // name's type is
                                     // const char[13]
f(name);          // name is array, but T deduced as const char*

// If instead

template<typename T>
void f(T& param);      // template with by-reference parameter

f(name);               // pass array to f, and T is actually deduced to be an array (const char [13])!
```

```cpp
// Functions, too, can decay into pointer

void someFunc(int, double);   // someFunc is a function;
                              // type is void(int, double)

template<typename T>
void f1(T param);             // in f1, param passed by value

template<typename T>
void f2(T& param);            // in f2, param passed by ref

f1(someFunc);                 // param deduced as ptr-to-func;
                              // type is void (*)(int, double)

f2(someFunc);                 // param deduced as ref-to-func;
                              // type is void (&)(int, double)
```

**Takeaway**

* During template type deduction, arguments that are references are treated as non-references, i.e., their reference-ness is ignored.
* When deducing types for universal reference parameters, lvalue arguments get special treatment.
* When deducing types for by-value parameters, const and/or volatile arguments are treated as non-const and non-volatile.
* During template type deduction, arguments that are array or function names decay to pointers, unless they’re used to initialize references.

### Understand auto type deduction

Auto type deduction actually largely follows the same rules as template type deduction.

```cpp
auto x = 27;          // case 3 (x is neither ptr nor reference)

const auto cx = x;    // case 3 (cx isn't either)

const auto& rx = x;   // case 1 (rx is a non-universal ref.)

// Case 2s:
auto&& uref1 = x;     // x is int and lvalue,
                      // so uref1's type is int&

auto&& uref2 = cx;    // cx is const int and lvalue,
                      // so uref2's type is const int&

auto&& uref3 = 27;    // 27 is int and rvalue,
                      // so uref3's type is int&&

// Same rule goes for arrays and functions

const char name[] =            // name's type is const char[13]
  "R. N. Briggs";

auto arr1 = name;              // arr1's type is const char*

auto& arr2 = name;             // arr2's type is
                               // const char (&)[13]


void someFunc(int, double);    // someFunc is a function;
                               // type is void(int, double)

auto func1 = someFunc;         // func1's type is
                               // void (*)(int, double)

auto& func2 = someFunc;        // func2's type is
                               // void (&)(int, double)
```

The only difference is in auto always treats {} as std::initializer\_list\<T\>, while template deduction does not.

```cpp
int x1 = 27;
int x2(27);
int x3 = { 27 };
int x4{ 27 };
// these four do the same thing

auto x1 = 27;             // type is int, value is 27

auto x2(27);              // ditto

auto x3 = { 27 };         // type is std::initializer_list<int>,
                          // value is { 27 }

auto x4{ 27 };            // ditto

// template type deduction does not work with {} as-is
auto x = { 11, 23, 9 };   // x's type is
                          // std::initializer_list<int>

template<typename T>      // template with parameter
void f(T param);          // declaration equivalent to
                          // x's declaration

f({ 11, 23, 9 });         // error! can't deduce type for T

// instead,
template<typename T>
void f(std::initializer_list<T> initList);

f({ 11, 23, 9 });         // T deduced as int, and initList's
                          // type is std::initializer_list<int>
```

**Takeaway**

* auto type deduction is usually the same as template type deduction, but auto type deduction assumes that a braced initializer represents a std::initializer_list, and template type deduction doesn’t.
* auto in a function return type or a lambda parameter implies template type deduction, not auto type deduction.

### Understand decltype

decltype rules differ from template deduction: it doesn't potentially drop referenceness, have special handling for universal reference or pass by value, instead it always spits back the type of the expression given to it.

In a typical use case where we use decltype to declare the return type of a function which is dependent upon template parameter types, we do

```cpp
template<typename Container, typename Index>    // C++11, works, but
auto authAndAccess(Container& c, Index i)       // requires
  -> decltype(c[i])                             // refinement
{
  authenticateUser();
  return c[i];
}
// the auto and -> indicates C++11's trailing return type to
// account for the fact that the types of c and i aren't known
// when we see the first auto. 


template<typename Container, typename Index>    // C++14;
auto authAndAccess(Container& c, Index i)       // not quite
{                                               // correct
  authenticateUser();
  return c[i];                  // return type deduced from c[i]
                                // using template type deduction rules
                                // (in this case by-value)
}
// Not quite right in the following sense:
std::deque<int> d;
authAndAccess(d, 0) = 10;  // return d[0], then assign 10 to it;
                           // this won't compile as referenceness
                           // is dropped


template<typename Container, typename Index>   // C++14; works,
decltype(auto)                                 // but still
authAndAccess(Container& c, Index i)           // requires
{                                              // refinement
  authenticateUser();
  return c[i];
}
// This works in the sense that the compiler is told to use decltype
// rules in lieu of template deduction rules for return type deduction.
// Needs refinement in the sense that c can only be lvalue ref.

// Similarly,
Widget w;

const Widget& cw = w;

auto myWidget1 = cw;             // auto type deduction:
                                 // myWidget1's type is Widget

decltype(auto) myWidget2 = cw;   // decltype type deduction:
                                 // myWidget2's type is
                                 //   const Widget&


template<typename Container, typename Index>       // final
decltype(auto)                                     // C++14
authAndAccess(Container&& c, Index i)              // version
{
  authenticateUser();
  return std::forward<Container>(c)[i];
}
// c can now be universal reference.


template<typename Container, typename Index>       // final
auto                                               // C++11
authAndAccess(Container&& c, Index i)              // version
  -> decltype(std::forward<Container>(c)[i])
{
  authenticateUser();
  return std::forward<Container>(c)[i];
}
```

decltype have few surprises, including the following
```
int x = 0;
decltype(x)   // yields int
decltype((x)) // yields int&, expr (x) is an lvalue expression, 
              // (not just a name) whose type is int&

// Expanding on this:
// In C++14, this is the kind of code that puts you on the
// express train to UB
decltype(auto) f1()
{
  int x = 0;
  return x;        // decltype(x) is int, so f1 returns int
}

decltype(auto) f2()
{
  int x = 0;
  return (x);      // decltype((x)) is int&, so f2 returns int&
}
```

**Takeaways**

* decltype almost always yields the type of a variable or expression without any modifications.
* For lvalue expressions of type T other than names, decltype always reports a type of T&.
* C++14 supports decltype(auto), which, like auto, deduces a type from its initializer, but it performs the type deduction using the decltype rules.

### Know how to view deduced types

IDE

Compiler diagnostics. Could rely on dummy code like this (trigger a compiler error to make it tell the types)
```cpp
// To view the types of x and y
template<typename T>       // declaration only for TD;
class TD;                  // TD == "Type Displayer"

TD<decltype(x)> xType;     // elicit errors containing
TD<decltype(y)> yType;     // x's and y's types
```

At runtime
```cpp
std::cout << typeid(x).name() << '\n';    // display types for x
// This approach relies on the fact that invoking typeid on an object
// such as x yields a std::type_info object, and std::type_info has a
// member function, name, that produces a C-style string
// (i.e., a const char*) representation of the name of the type.

// It may show something like a PKi, pointer to const integer (this
// display can be demangled)

// std::type_info::name could be incorrect, because the specification
// for std::type_info::name mandates that the type be treated as if it
// had been passed to a template function as a by-value parameter.
```

Where IDE and std::type_info::name could be wrong, Boost.TypeIndex is designed to be correct.

**Takeaways**
* Deduced types can often be seen using IDE editors, compiler error messages, and the Boost TypeIndex library
* The results of some tools may be neither helpful nor accurate, so an understanding of C++’s type deduction rules remains essential

