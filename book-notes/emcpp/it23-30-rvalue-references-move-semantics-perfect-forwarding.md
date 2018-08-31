# Chap 5. Rvalue references, move semantics, and perfect forwarding

std::unique\_ptr, std::future and std::thread are move-only.

Perfect forwarding makes it possible to write function templates that take arbitrary arguments and forward them to other functions such that the target functions receive exactly the same arguments as were passed to the forwarding functions.

Rvalue references make both move semantics and perfect forwarding possible.

A parameter is always an lvalue, no matter its type (rvalue reference, lvalue, etc)

### Understand std::move and std::forward

Neither std::move nor std::forward generates code at runtime.
They are function templates that perform casts.
std::move unconditionally casts its argument to an rvalue, while std::forward performs this cast only if a particular condition is fulfilled.

std::move looks something like this:
```cpp
// C++11
template<typename T>                       // in namespace std
typename remove_reference<T>::type&&
move(T&& param)
{
  using ReturnType =                       // alias declaration;
    typename remove_reference<T>::type&&;  // see Item 9

  return static_cast<ReturnType>(param);
}

// C++14
template<typename T>                          // still in
decltype(auto) move(T&& param)                // namespace std
{
  using ReturnType = remove_reference_t<T>&&;
  return static_cast<ReturnType>(param);
}
```
The remove\_reference in return type is important since type&& is a universal reference (in which case if T is an lvalue reference the universal reference would become an lvalue reference.
To make move truly return a rvalue reference, we have this remove\_reference.

Point is, std::move does rvalue\_cast, not move.
rvalues are candidates for move, by casting to rvalue reference, it tells compiler the object is eligible to be moved from.

Consider this code,
```cpp
class Annotation {
public:
  explicit Annotation(const std::string text)
  : value(std::move(text))  // "move" text into value; this code
  { … }                     // doesn't do what it seems to!
  
  …

private:
  std::string value;
};
```
Is text copied or moved to value?
It is copied due to the fact that the rvalue of const string can't be passed to std::string's move ctor, as move ctors need non-const std::string.
However the param of string copy ctor, lvalue-reference-to-const, can bind to a const rvalue.
Thus the string is copied from text to value.

Moral of the story: move doesn't move, and if you want to be able to move from something, don't declare it const.

std::forward is a conditional cast. Consider this code
```cpp
void process(const Widget& lvalArg);     // process lvalues
void process(Widget&& rvalArg);          // process rvalues

template<typename T>                     // template that passes
void logAndProcess(T&& param)            // param to process
{
  auto now =                             // get current time
    std::chrono::system_clock::now();

  makeLogEntry("Calling 'process'", now);
  process(std::forward<T>(param));
}

...

Widget w;

logAndProcess(w);                  // call with lvalue
logAndProcess(std::move(w));       // call with rvalue
```
Without the std::forward, since param is an lvalue, the overload expecting lvalue will always be called.
To forward the rvalue/lvalue-ness of a parameter, we use std::forward.

std::forward casts param into an rvalue, only if param is instantiated with an rvalue. 

Can you replace std::move with std::forward everywhere?
Technically yes. And neither is really necessary as you can write casts everywhere, just not desirable.
But remember they are different in a mandatory rvalue cast and conditional rvalue cast (that's exactly a forward of the rvalue/lvalue-ness of the object the function param is instantiated with)

**Takeaways**
* std::move performs an unconditional cast to an rvalue. In and of itself, it doesn’t move anything.
* std::forward casts its argument to an rvalue only if that argument is bound to an rvalue.
* Neither std::move nor std::forward do anything at runtime.
* Move requests on const objects are treated as copy requests.

### Distinguish universal references from rvalue references

If you see T&& in source code, it's not always rvalue reference.

```cpp
void f(Widget&& param);             // rvalue reference

Widget&& var1 = Widget();           // rvalue reference

auto&& var2 = var1;                 // not rvalue reference

template<typename T>
void f(std::vector<T>&& param);     // rvalue reference

template<typename T>
void f(T&& param);                  // not rvalue reference
```

T&& has two meanings.
One meaning is rvalue reference, to identify objects that may be moved from.
Another meaning is universal reference, either rvalue or lvalue reference, they are permitted to bind to rvalues or lvalues, const or non const, volatile or non volatile.

Universal references arise in template parameters and auto declarations.
```cpp
template<typename T>
void f(T&& param);             // param is a universal reference

auto&& var2 = var1;            // var2 is a universal reference
```
Both of these have template type deduction.
When there isn't type deduction happening, && is an rvalue reference.

References have to be initialized.
The initializer for universal references decide if it represents an rvalue reference or an lvalue reference: if initializer is rvalue, it's rvalue reference; if it's lvalue, it's lvalue references. E.g.

```cpp
template<typename T>
void f(T&& param);     // param is a universal reference

Widget w;
f(w);                  // lvalue passed to f; param's type is
                       // Widget& (i.e., an lvalue reference)

f(std::move(w));       // rvalue passed to f; param's type is
                       // Widget&& (i.e., an rvalue reference)
```

Universal reference must precisely be T&&, can't even be the likes of const T&& or vector<T>&&; and type deduction has to happen. Consider this:
```cpp
template<class T, class Allocator = allocator<T>>  // from C++
class vector {                                     // Standards
public:
  void push_back(T&& x);
  …
  template <class... Args>
  void emplace_back(Args&&... args);
};
```

The argument x is not a universal reference, since push\_back can’t exist without a particular vector instantiation for it to be part of, and the type of that instantiation fully determines the declaration for push_back.

Consider this:
```cpp
std::vector<Widget> v;
...
class vector<Widget, allocator<Widget>> {
public:
  void push_back(Widget&& x);               // rvalue reference
  …
};
```

The arguments args are universal references, since Args are independent of T.

Variables declared with the type auto&& are universal references, because type deduction takes place and they have the correct form (“T&&”).
One use case is such in C++14 where auto&& is allowed as lambda parameters.
```cpp
auto timeFuncInvocation =
  [](auto&& func, auto&&... params)               // C++14
  {
    start timer;
    std::forward<decltype(func)>(func)(           // invoke func
      std::forward<decltype(params)>(params)...   // on params
      );                              
    stop timer and record elapsed time;
  };
```

The underlying is actually reference collapsing, which we'll get to later.

**Takeaways**
* If a function template parameter has type T&& for a deduced type T, or if an object is declared using auto&&, the parameter or object is a universal reference.
* If the form of the type declaration isn’t precisely type&&, or if type deduction does not occur, type&& denotes an rvalue reference.
* Universal references correspond to rvalue references if they’re initialized with rvalues. They correspond to lvalue references if they’re initialized with lvalues.  

### Use std::move on rvalue references, std::forward on universal references

If a function has an rvalue reference parameter, you know the object's bound to may be moved.

```cpp
class Widget {
  Widget(Widget&& rhs);        // rhs definitely refers to an
  …                            // object eligible for moving
};
```

That being the case, you'll want to pass such objects to other functions in a way that permits those functions to take advantage of the object's rvalueness.
The way to do it is to cast parameters to rvalues using std::move. E.g.

```cpp
class Widget {
public:
  Widget(Widget&& rhs)               // rhs is rvalue reference
  : name(std::move(rhs.name)),
    p(std::move(rhs.p))
    { … }
  …

private:
  std::string name;
  std::shared_ptr<SomeDataStructure> p;
};
```

A universal reference might be bound to an object that's eligible for moving: they can be casted to rvalues only if they were intialized with rvalues.
This is precisely what std::forward does.

```cpp
class Widget {
public:
  template<typename T>
  void setName(T&& newName)               // newName is
  { name = std::forward<T>(newName); }    // universal reference

  …
};
```

Don't use std::forward on rvalue references, and more importantly, don't use std::move on universal references, you may unexpectedly modify lvalues.
```cpp
class Widget {
public:
  template<typename T>
  void setName(T&& newName)         // universal reference
  { name = std::move(newName); }    // compiles, but is
  …                                 // bad, bad, bad!

private:
  std::string name;
  std::shared_ptr<SomeDataStructure> p;
};

std::string getWidgetName();        // factory function

Widget w;

auto n = getWidgetName();           // n is local variable

w.setName(n);                       // moves n into w!

…                                   // n's value now unknown
```

Consider the following two:
```cpp
// Widget class, taking universal reference
  template<typename T>
  void setName(T&& newName)         // universal reference
  { name = std::move(newName); }    // compiles, but is
  …                                 // bad, bad, bad!

// You can have these as an alternative
  void setName(const std::string& newName)      // set from
  { name = newName; }                           // const lvalue

  void setName(std::string&& newName)           // set from
  { name = std::move(newName); }                // rvalue

// Problem with the alternative is that (despite very minor efficiency concerns when passing a string literal) when the method takes N (or even unlimited) parameters you'd have 2^N overloads.

// Like these guys:
template<class T, class... Args>                 // from C++11
shared_ptr<T> make_shared(Args&&... args);       // Standard

template<class T, class... Args>                 // from C++14
unique_ptr<T> make_unique(Args&&... args);       // Standard

// And inside such functions, I assure you, std::forward is applied to the universal reference parameters when they’re passed to other functions. Which is exactly what you should do (eventually, after you are done with it in the function body, like the following example)

template<typename T>                       // text is
void setSignText(T&& text)                 // univ. reference
{
  sign.setText(text);                      // use text, but
                                           // don't modify it

  auto now =                               // get current time
    std::chrono::system_clock::now();
    
  signHistory.add(now,
                  std::forward<T>(text));  // conditionally cast
}                                          // text to rvalue

// In rare cases, you’ll want to call std::move_if_noexcept instead of std::move.
```

If you are in a function returning by value, and you are returning an object bound to an rvalue reference or a universal reference, you'll want to apply std::move or std::forward when you return the reference.
Consider the following
```cpp
Matrix                                        // by-value return
operator+(Matrix&& lhs, const Matrix& rhs)
{
  lhs += rhs;
  return std::move(lhs);                      // move lhs into
}                                             // return value

Matrix                                        // as above
operator+(Matrix&& lhs, const Matrix& rhs)
{
  lhs += rhs;
  return lhs;                                 // copy lhs into
}                                             // return value
// the first approach works as-is if Matrix supports move construction. If not,
// casting it to rvalue won't hurt as the rvalue will be copied by Matrix's copyctor

// Similar case goes for using std::forward on universal references.
```

But should you do this with the case of returning local variables by value?
```cpp
Widget makeWidget()        // "Copying" version of makeWidget
{
  Widget w;                // local variable

  …                        // configure w

  return w;                // "copy" w into return value
}

Widget makeWidget()        // Moving version of makeWidget
{
  Widget w;
  …
  return std::move(w);     // move w into return value
}                          // (don't do this!)
```
Return value optimization does this for you, and is in the standards.
With the std::move, RVO cannot be applied as it requires 1) type of the local object is the same as that returned by the function, 2) the local object is what's being returned.
When you do std::move in this case, you are returning a reference instead of the object itself, which breaks the condition for RVO.

Since RVO is not required, what if you suspect the compiler doesn't do it, or you know the internals of this function is probably too hard for the compiler to apply RVO?
Still don't do this, as the standards say if the conditions of RVO are met but the compiler does not do it, the compiler is still required to treat the returned local variable as an rvalue.

Using std::move on a local variable could be useful, if you know later on you aren't going to use it, but not in this return local object by value case.

**Takeaways**
* Apply std::move to rvalue references and std::forward to universal references the last time each is used.
* Do the same thing for rvalue references and universal references being returned from functions that return by value.
* Never apply std::move or std::forward to local objects if they would otherwise be eligible for the return value optimization.

