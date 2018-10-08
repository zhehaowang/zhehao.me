# Accustoming yourself with C++

### View C++ as a federation of languages

View C++ as a federation of languages.
The rules within each particular sublanguage tend to be straightforward, however, the rules change as you move from one sublanguage to another.

The primary sublanguages are
* C. Statements, blocks, preprocessor, built-in types, arrays, pointers, etc. C++ often offers superior approaches to the C counterparts
* OO C++. Classes, encapsulation, inheritance, polymorphism, dynamic binding, etc
* Template C++. The aspect of generic programming in C++
* The STL. Containers, iterators, algorithms, function objects that mesh beautifully.

Don't be surprised when moving from one sublanguage to another, the strategy might switch.
For example, pass built-in C types by value, but pass user defined C++ classes by reference to const are often times more efficient.

**Takeaways**
* Rules for effective C++ programming vary, depending on the part of C++ you are using.

### Prefer const, enum, inline, to define

Alternatively, this item can be called "prefer compilers to preprocessors"

\#define is substituted by the preprocessor thus the symbol is never seen by the compiler, nor does it exist inside symbol table.
A language const instead will be seen by the compiler.

Advantages of using the latter include
* The compiler will be able to point out the symbol should an error occur
* If the const appears multiple times, the memory footprint is smaller using a const (only one copy in memory)
* No way to create class-specific const with \#define because define does not respect scope

When declaring const to replace \#define, keep in mind that pointers should have both the pointer and the object it refers to declared const. E.g.
```cpp
const char* const author = "me";
```
Though item 3 would claim in this case, a const std::string is preferable
```cpp
const std::string author = "me";
```

Another case to keep in mind is class-specific consts, to limit the scope of a constant to a class, you should make it a member, to ensure there is only one copy, make it a static member.
```cpp
class GamePlayer {
private:
  static const int NumTurns = 5;      // constant declaration
  int scores[NumTurns];               // use of constant
  ...
};
```
If you need to take the address of NumTurns, define it in the impl file of this class like this
```cpp
#include <game_player.h>
...
const int GamePlayer::NumTurns;  // definition
```
This does not go in the header so that you don't double define when the header is included in multiple places, and the definition does not have '=' since an initial value is already given at the point of declaration.

In-class initialization is allowed only for integral types and only for constants.
In cases where the above syntax can't be used, you put the initial value at the point of definition. Like this
```cpp
class CostEstimate {
private:

  static const double FudgeFactor;       // declaration of static class
  ...                                    // constant; goes in header file
};
const double                             // definition of static class
  CostEstimate::FudgeFactor = 1.35;      // constant; goes in impl. file
```

If your compiler wrongfully forbid the in-class initialization of constant integral types, which is required to, for example, declare an array of that size, you could do the "enum hack". Like this
```cpp
class GamePlayer {
private:
  enum { NumTurns = 5 };        // "the enum hack" — makes
                                // NumTurns a symbolic name for 5

  int scores[NumTurns];         // fine

  ...

};
```
The enum hack can also be used to forbid your client caller from taking the address or a reference of 'NumTurns', or to enforce that compiler does not allocate memory for 'NumTurns' (due to its being unnecessary).
And enum hack is fundamental for TMP.

Another common misuse of \#define is to implement macros that look like functions but don't incur the cost of a function call.
This misuse can be confusing to reason, e.g.
```cpp
#define CALL_WITH_MAX(a, b) f((a) > (b) ? (a) : (b))

int a = 5, b = 0;

CALL_WITH_MAX(++a, b);          // a is incremented twice
CALL_WITH_MAX(++a, b+10);       // a is incremented once
```
Here, the number of times that a is incremented before calling f depends on what it is being compared with!

Do this instead with a regular inline function using templates.
```cpp
template<typename T>                               // because we don't
inline void callWithMax(const T& a, const T& b)    // know what T is, we
{                                                  // pass by reference-to-
  f(a > b ? a : b);                                // const — see Item 20
}
```

Given the availability of consts, enums, and inlines, your need for the preprocessor (especially \#define) is reduced, but not eliminated.
\#include, \#ifdef, \#ifndef continue to play important roles in controlling compilation.

**Takeaways**
* For simple constants, prefer const objects or enums to #defines.
* For function-like macros, prefer inline functions to #defines.

### Use const whenever possible

const is how you can communicate to the compiler and other programmers that a value should not be altered and the compiler will enforce it. Use it whenever this constraint holds.

Using const with pointers:
```cpp
char greeting[] = "Hello";

char *p = greeting;                    // non-const pointer,
                                       // non-const data

const char *p = greeting;              // non-const pointer,
                                       // const data

char * const p = greeting;             // const pointer,
                                       // non-const data

const char * const p = greeting;       // const pointer,
                                       // const data
```
If the word const appears to the left of the asterisk, what's pointed to is constant; if the word const appears to the right of the asterisk, the pointer itself is constant; if const appears on both sides, both are constant.
If a const appears on the left of the asterisk, whether it's "const type" or "type const" makes no difference.

An STL iterator is modeled on a pointer, declaring an iterator itself const will be analogous to declaring the pointer const.
If with STL you want to declare the data const, use const\_iterator.

```cpp
std::vector<int> vec;
...

const std::vector<int>::iterator iter =     // iter acts like a T* const
  vec.begin();
*iter = 10;                                 // OK, changes what iter points to
++iter;                                    // error! iter is const

std::vector<int>::const_iterator cIter =   //cIter acts like a const T*
  vec.begin();
*cIter = 10;                               // error! *cIter is const
++cIter;                                  // fine, changes cIter
```

const can be used to specify a function return value, its parameters, and the function itself if it is a member function.

Having a function return a constant value is generally inappropriate, but sometimes doing so can reduce the incidence of client errors without giving up safety or efficiency. For example,
```cpp
class Rational { ... };

const Rational operator*(const Rational& lhs, const Rational& rhs);

// why should the operator* for Rationals return const?
// because if not, clients can inadvertently, do

Rational a, b, c;

(a * b) = c;                           // invoke operator= on the
                                       // result of a*b!

// and all it takes is a typo for the above code to happen

if (a * b = c) ...                     // oops, meant to do a comparison!
```
One of the hallmarks of good user-defined types is that they avoid gratuitous incompatibilities with the built-ins.
The above where the product of two can be assigned is to is pretty gratuitous.

const member functions are important as they make the interface easier to understand (which functions can change the object in question), and they make it possible to work with const objects.

Member functions differing only in their constness can be overloaded. E.g.
```cpp
class TextBlock {
  public:
    ...
    const char&                                       // operator[] for
    operator[](const std::size_t position) const      // const objects
    { return text[position]; }
 
    char&                                             // operator[] for
    operator[](const std::size_t position) const      // non-const objects
    { return text[position]; }
  
  private:
    std::string text;
};
```

It's never legal to modify the return value of a function that returns a built-in type. (and when modify the return value of a function, note that it'd be done on a copy of the source inside that function)

Semantics-wise, there is bitwise constness and logical constness.
C++ uses bitwise constness, where a const member function is not allowed to modify any of the bits inside the object.

Bitwise const would mean if the object's data member is a pointer, inside a const member function where the pointer looks at cannot be changed, but the content of what it points to can be. For example,
```cpp
class CTextBlock {
public:
  ...

  char& operator[](std::size_t position) const   // inappropriate (but bitwise
  { return pText[position]; }                    // const) declaration of
                                                 // operator[]
private:
  char *pText;
};
// this compiles without issues.

const CTextBlock cctb("Hello");        // declare constant object

char *pc = &cctb[0];                   // call the const operator[] to get a
                                       // pointer to cctb's data

*pc = 'J';                              // cctb now has the value "Jello"
// in the book this should allow you to change the value, though in my code
// sample the assignment call results in 'bus error'
```

Logical constness suggest that a const member function may modify parts of the object only if its clients cannot detect.
This notion can be achieved with 'mutable' keyword. E.g.
```cpp
class CTextBlock {
public:

  ...

  std::size_t length() const;

private:
  char *pText;

  mutable std::size_t textLength;         // these data members may
  mutable bool lengthIsValid;             // always be modified, even in
};                                        // const member functions

std::size_t CTextBlock::length() const
{
  if (!lengthIsValid) {
    textLength = std::strlen(pText);      // now fine
    lengthIsValid = true;                 // also fine
  }

  return textLength;
}
```

Now suppose you have boundary check, logging, etc in TextBlock's operator[].
These logic would be duplicated in both the const and the non-const version.
To avoid the duplication, we could do a cast instead (which is usually a bad idea in other circumstances) like this
```cpp
class TextBlock {
public:
  const char& operator[](std::size_t position) const     // same as before
  {
    ... // some shared operations
    return text[position];
  }

  char& operator[](std::size_t position)         // now just calls const op[]
  {
    return
      const_cast<char&>(                         // cast away const on
                                                 // op[]'s return type;
        static_cast<const TextBlock&>(*this)     // add const to *this's type;
          [position]                             // call const version of op[]
      );
  }
};
```
This is safe as when we are given a non-const TextBlock, we can safely invoke the const version and then cast its result.
The other way round of having the const version call the non-const version is not something you want to do.

**Takeaways**
* Declaring something const helps compilers detect usage errors. const can be applied to objects at any scope, to function parameters and return types, and to member functions as a whole.
* Compilers enforce bitwise constness, but you should program using conceptual constness.
* When const and non-const member functions have essentially identical implementations, code duplication can be avoided by having the non-const version call the const version.

### Make sure objects are initialized before they are used


