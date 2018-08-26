# Moving to Modern C++

### Distinguish between () and {} when creating objects

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

Why not always use braced initialization then? There can be unexpected behaviors due to the tangled relationship among initializers, std::intializer_lists, and constructor overload resolution.
E.g. in [item 2](it1-4-deducing-types.md#understand-auto-type-deduction), deduced type for auto using braced initialization is std::initializer_list. (The more you like auto, the less you may like braced initialization)

```cpp
// Braced initialization always prefers a constructor overload that takes in std::initialization_list,
// and what would normally be copy and move construction can be hijacked by std::initialization_list ctors
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
Consider std::vector, it has a (length, value) ctor and a std::initializer_list ctor.
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
That means as a class designer, don't do what std::vector does.
It’s best to design your constructors so that the overload called isn’t affected by whether clients use parentheses or braces.
And if you need to add a std::initializer_list ctor, do so with great deliberation.

The second lesson is that as a class client, you must choose carefully between parentheses and braces when creating objects.

**Takeaway**
* Braced initialization is the most widely usable initialization syntax, it prevents narrowing conversions, and it’s immune to C++’s most vexing parse.
* During constructor overload resolution, braced initializers are matched to std::initializer_list parameters if at all possible, even if other constructors offer seemingly better matches.
* An example of where the choice between parentheses and braces can make a significant difference is creating a std::vector<numeric type> with two arguments.
* Choosing between parentheses and braces for object creation inside templates can be challenging. One could do braces only when necessary; or instead, always do {} but understand when semantically () is desirable


### Prefer nullptr to 0 and NULL

Neither 0 or null has a pointer type.
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

nullptr does not have an integral type. It does not suffer from overload resolution surprises that 0 and NULL are susceptible to.
It doesn't have a pointer type either, but you can think of it as a pointer of all types.
Its actual type is std::nullptr_t, which implicitly converts to all pointer types.

nullptr improves code clarity. Consider
```cpp
auto result = findRecord( /* arguments */ );

if (result == nullptr) {
  …
}

// vs
auto result = findRecord( /* arguments */ );

if (result == 0) {
  …
}
```

nullptr shines even more in template specialization.

**Takeaways**
* Prefer nullptr to 0 and NULL.
* Avoid overloading on integral and pointer types.

### Prefer alias declarations to typedefs

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
// because MyAllocList<T>::type is now a dependent type. (Compiler doesn't
// know for sure MyAllocList<T>::type is a type. There might be a
// specialization of MyAllocList<T> somewhere that has ::type not as a type,
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
* typedefs don’t support templatization, but alias declarations do.
* Alias templates avoid the “::type” suffix and, in templates, the “typename” prefix often required to refer to typedefs.
* C++14 offers alias templates for all the C++11 type traits transformations.

### Prefer scoped enums to unscoped enums

As a general rule, declaring a name inside curly braces limits the visibility of that name to the scope defined by the braces. Not so for the enumerators declared in C++98-style enums.
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

Scoped enums are declared via enum class, and they are referred to as enum classes as well.

Enum classes
* Reduce namespace pollution
* Are strongly typed (no implicit conversion to other types, while unscoped enum implicitly convert to integral types)

Enum classes can be forward declared (by default the underlying type for scoped enums in int, so the compiler knows the size of a forward declared enum. You can override the default underlying type).
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
// so you could have
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
* Both scoped and unscoped enums support specification of the underlying type. The default underlying type for scoped enums is int. Unscoped enums have no default underlying type.
* Scoped enums may always be forward-declared. Unscoped enums may be forward-declared only if their declaration specifies an underlying type.

### Prefer deleted functions to private undefined ones

You often want to suppress the special member functions that the compiler generates for you, like copycon and assignment operator.
In C++03 you do that with private undefined copycon and assignment opr.
In C++10 you do that with declaration with "= delete".

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
The advantages of delete
* delete will result in better error messages: always at compile time (as opposed to friends / members seeing undefined symbols)
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

### Declare overriding functions override

