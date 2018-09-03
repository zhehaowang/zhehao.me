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

### Avoid overloading on universal references

Consider this code

```cpp
std::multiset<std::string> names;     // global data structure

void logAndAdd(const std::string& name)
{
  auto now =                          // get current time
    std::chrono::system_clock::now();

  log(now, "logAndAdd");              // make log entry

  names.emplace(name);                // add name to global data
}                                     // structure; see Item 42
                                      // for info on emplace

std::string petName("Darla");

logAndAdd(petName);                   // pass lvalue std::string
// a copy is made in the vector since lvalue is passed in

logAndAdd(std::string("Persephone")); // pass rvalue std::string
// a copy is made in the vector since 'name' is an lvalue.
// just a move in this case is possible.

logAndAdd("Patty Dog");               // pass string literal
// a string object is first created from the string literal,
// then a copy is made in the vector since 'name' is an lvalue
// not even a move is needed in this case: if the string literal is
// given to emplace directly, emplace would have used it directly
// to create the string object inside the vector.
```

Instead, for optimal efficiency, we could have this
```cpp
template<typename T>
void logAndAdd(T&& name)
{
  auto now = std::chrono::system_clock::now();
  log(now, "logAndAdd");
  names.emplace(std::forward<T>(name));
}

std::string petName("Darla");          // as before

logAndAdd(petName);                    // as before, copy
                                       // lvalue into multiset

logAndAdd(std::string("Persephone"));  // move rvalue instead
                                       // of copying it

logAndAdd("Patty Dog");                // create std::string
                                       // in multiset instead
                                       // of copying a temporary
                                       // std::string
```

Then, consider the case where clients want an overload of logAndAdd taking in int
```cpp
std::string nameFromIdx(int idx);      // return name
                                       // corresponding to idx

void logAndAdd(int idx)                // new overload
{
  auto now = std::chrono::system_clock::now();
  log(now, "logAndAdd");
  names.emplace(nameFromIdx(idx));
}

std::string petName("Darla");          // as before

logAndAdd(petName);                    // as before, these
logAndAdd(std::string("Persephone"));  // calls all invoke
logAndAdd("Patty Dog");                // the T&& overload

logAndAdd(22);                         // calls int overload

// but:
short nameIdx;
…                                      // give nameIdx a value

logAndAdd(nameIdx);                    // error!
// the given short matches the universal reference version as short&,
// (better match than the int overload)
// thus an emplace to vector<string> is called given a short&, and
// it's an error since there isn't ctor of string that takes a short.
```

Functions taking universal references are the greediest functions in C++.
They instantiate to create exact matches for almost any type of argument.

This is why combining overloading and universal references is almost always a bad idea: the universal reference overload vacuums up far more argument types than the developer doing the overloading generally expects.

In a similar example with class ctors
```cpp
class Person {
public:
  template<typename T>              // perfect forwarding ctor
  explicit Person(T&& n)
  : name(std::forward<T>(n)) {}

  explicit Person(int idx);         // int ctor

  // despite having the perfect forwarding ctor, compiler supplies
  // copy and move ctor following the rules of it17
  Person(const Person& rhs);        // copy ctor
                                    // (compiler-generated)

  Person(Person&& rhs);             // move ctor
  …                                 // (compiler-generated)

};

// and this would cause:
Person p("Nancy");

auto cloneOfP(p);                   // create new Person from p;
                                    // this won't compile!
// the perfect forwarding ctor will be called, and string does not
// exist a ctor taking in a Person.
// why the perfect forwarding ctor, not the copy ctor? Because the
// perfect forwarding ctor (taking in Person&) is a perfect match
// while the copy ctor requires adding const

// change it up a bit:
const Person cp("Nancy");     // object is now const

auto cloneOfP(cp);            // calls copy constructor!
// the perfect forwarding will be instantiated with
  explicit Person(const Person& n);      // instantiated from
                                         // template
// but this doesn't matter, as one of the overload-resolution rules
// in C++ is that in situations where a template instantiation and
// a non-template function (i.e., a “normal” function) are equally
// good matches for a function call, the normal function is preferred.
```

And with inheritance,
```cpp
class SpecialPerson: public Person {
public:
  SpecialPerson(const SpecialPerson& rhs)  // copy ctor; calls
  : Person(rhs)                            // base class
  { … }                                    // forwarding ctor!

  SpecialPerson(SpecialPerson&& rhs)       // move ctor; calls
  : Person(std::move(rhs))                 // base class
  { … }                                    // forwarding ctor!
};
// note that the derived class's copy and move ctors don't call
// their base class's copy and ctor, they call the base class's
// perfect-forwarding ctor!
// To understand why, note that the derived class functions are using
// arguments of type SpecialPerson to pass to their base class.
```

**Takeaways**
* Overloading on universal references almost always leads to the universal reference overload being called more frequently than expected.
* Perfect-forwarding constructors are especially problematic, because they’re typically better matches than copy constructors for non-const lvalues, and they can hijack derived class calls to base class copy and move constructors.

### Familiarize yourself with alternatives to overloading on universal references

Avoid overloading is one option.

Pass by "const T&" could sacrifice a little efficiency, but avoids a universal reference overload.

Pass by value is counter intuitive, but if you know you'll copy them, this dials up performance without any increase in complexity. (Item 41) E.g.
```cpp
class Person {
public:
  explicit Person(std::string n) // replaces T&& ctor; see
  : name(std::move(n)) {}        // Item 41 for use of std::move
  
  explicit Person(int idx)       // as before
  : name(nameFromIdx(idx)) {}
  …

private:
  std::string name;
};
```

None of the above has support for perfect forwarding.
If we want perfect forwarding we have to use universal references, and to use that with overload we can do tag dispatch.
We start with this universal reference version which is problematic in the face of overload:
```cpp
template<typename T>                   // make log entry and add
void logAndAdd(T&& name)               // name to data structure
{
  auto now = std::chrono::system_clock::now();
  log(now, "logAndAdd");
  names.emplace(std::forward<T>(name));
}
```
We split the underlying work into two functions
```cpp
template<typename T>
void logAndAdd(T&& name)
{
  logAndAddImpl(std::forward<T>(name),
                std::is_integral<T>());     // not quite correct
}
// Why not quite correct? If lvalue reference is passed in T is int& and
// that is not an integral type.

// Instead we have
template<typename T>
void logAndAdd(T&& name)
{
  logAndAddImpl(
    std::forward<T>(name),
    std::is_integral<typename std::remove_reference<T>::type>()
    // In C++14, do "std::remove_reference_t<T>"
  );
}
// And the impl looks like
template<typename T>                             // non-integral
void logAndAddImpl(T&& name, std::false_type)    // argument:
{                                                // add it to
  auto now = std::chrono::system_clock::now();   // global data
  log(now, "logAndAdd");                         // structure
  names.emplace(std::forward<T>(name));
}
// std::false_type is a compile-time type that corresponds to false

// and the second overload
std::string nameFromIdx(int idx);             // as in Item 26

void logAndAddImpl(int idx, std::true_type)   // integral
{                                             // argument: look
  logAndAdd(nameFromIdx(idx));                // up name and
}                                             // call logAndAdd
                                              // with it
// this technique is called tag dispatch.
// We hope that compilers will recognize the 2nd parameter serves
// no purpose at runtime and will optimize them out.
```

A keystone of tag dispatch is the existence of a single (unoverloaded) function as the client API.

But in the case of ctors, compilers supply their own even if you only write one ctor taking universal references.
For situations like these, where an overloaded function taking a universal reference is greedier than you want, yet not greedy enough to act as a single dispatch function, tag dispatch is not the droid you’re looking for.
In this case you need std::enable\_if.
enable\_if gives you a way to force compilers to behave as if a particular template didn't exist. E.g.
```cpp
class Person {
public:
  template<typename T,
           typename = typename std::enable_if<condition>::type>
  explicit Person(T&& n);

  …
};
// SFINAE is the technology that makes std::enable_if work.
// In our case, we want to disable the universal reference ctor only if T,
// with cv qualifier and reference-ness dropped, is the same as Person.
// To drop cv qualifier and reference-ness, we use std::decay<T>

// Thus we have
class Person {
public:
  template<
    typename T,
    typename = typename std::enable_if<
                 !std::is_same<Person,
                               typename std::decay<T>::type
                              >::value
               >::type
  >
  explicit Person(T&& n);
  …
};
```
That addresses the problem of Person class, but in Item 26 there is the issue of derived classes calling base's universal reference ctor instead of base's corresponding copy or move ctor.
To address that, we have
```cpp
class Person {
public:
  template<
    typename T,
    typename = typename std::enable_if<
                 !std::is_base_of<Person,
                                  typename std::decay<T>::type
                                 >::value
               >::type
  >
  explicit Person(T&& n);
  // note that for user defined type T std::is_base_of<T, T> is true.
  // for built-in types it's false.
  …
};

// and the same in C++14:
class Person {                                     // C++14
public:
  template<
    typename T,
    typename = std::enable_if_t<               // less code here
                 !std::is_base_of<Person,
                                  std::decay_t<T>  // and here
                                 >::value
               >                                   // and here
  >
  explicit Person(T&& n);
  …
};

// combining this with the integral type exclusion we solved with tag dispatch earlier:
class Person {
public:
  template<
    typename T,
    typename = std::enable_if_t<
      !std::is_base_of<Person, std::decay_t<T>>::value
      &&
      !std::is_integral<std::remove_reference_t<T>>::value
    >
  > 
  explicit Person(T&& n)        // ctor for std::strings and
  : name(std::forward<T>(n))    // args convertible to
  { … }                         // std::strings

  explicit Person(int idx)      // ctor for integral args
  : name(nameFromIdx(idx))
  { … }

  …                             // copy and move ctors, etc.

private:
  std::string name;
};
// this uses perfect forwarding and should offer maximal efficiency, and with universal
// references controlled this technique can be used in circumstances where overloading
// is unavoidable
```

As a rule, perfect forwarding is more efficient, because it avoids the creation of temporary objects solely for the purpose of conforming to the type of a parameter declaration.
In the case of the Person constructor, perfect forwarding permits a string literal such as "Nancy" to be forwarded to the constructor for the std::string inside Person, whereas techniques not using perfect forwarding must create a temporary std::string object from the string literal to satisfy the parameter specification for the Person constructor.

But perfect forwarding has drawbacks, one is some kinds of arguments can't be perfect-forwarded.
Another is the comprehensibility of compiler error messages.
To combat the comprehensibility issue, we can do static\_assertion
```cpp
class Person {
public:
  template<                                 // as before
    typename T,
    typename = std::enable_if_t<
      !std::is_base_of<Person, std::decay_t<T>>::value
      &&
      !std::is_integral<std::remove_reference_t<T>>::value
    >
  >
  explicit Person(T&& n)
  : name(std::forward<T>(n))
  {
    // assert that a std::string can be created from a T object
    static_assert(
      std::is_constructible<std::string, T>::value,
      "Parameter n can't be used to construct a std::string"
   );

   …                    // the usual ctor work goes here

  }

  …                     // remainder of Person class (as before)

};
// unfortunately in this case the static_assert being in function body after
// member initialization list would cause the long message to be printed
// first.
```

**Takeaways**
* Alternatives to the combination of universal references and overloading include the use of distinct function names, passing parameters by lvalue-reference-to-const, passing parameters by value, and using tag dispatch.
* Constraining templates via std::enable_if permits the use of universal references and overloading together, but it controls the conditions under which compilers may use the universal reference overloads.
* Universal reference parameters often have efficiency advantages, but they typically have usability disadvantages.

### Understand reference collapsing

Consider template parameter deduction for universal references again.
```cpp
template<typename T>
void func(T&& param);

Widget widgetFactory();     // function returning rvalue

Widget w;                   // a variable (an lvalue)

func(w);                    // call func with lvalue; T deduced
                            // to be Widget&

func(widgetFactory());      // call func with rvalue; T deduced
                            // to be Widget
```

Note that reference to reference is illegal in C++
```cpp
int x;
…
auto& & rx = x;   // error! can't declare reference to reference

// but with lvalue being deduced to match universal references
template<typename T>
void func(T&& param);    // as before

func(w);                 // invoke func with lvalue;
                         // T deduced as Widget&
// we would have this
void func(Widget& && param);
// yet the type of param is Widget&. How compiler handles this is called
// reference collapsing.
```

Compilers allow reference to reference not in user code but may produce them in some contexts, e.g. template instantiation.

There are four possible combinations: lvalue to lvalue, lvalue to rvalue, rvalue to lvalue, rvalue to rvalue.
If reference to reference occurs in a permitted context, compiler follows the following rule to collapse reference:

If either reference is an lvalue reference, the result is an lvalue reference. Otherwise (i.e., if both are rvalue references) the result is an rvalue reference.

Reference collapsing is a key part of what makes std::forward work. Whose impl can be
```cpp
template<typename T>                                // in
T&& forward(typename                                // namespace
              remove_reference<T>::type& param)     // std
{
  return static_cast<T&&>(param);
}
```

Reference collapsing can be mimicked in auto form,
```cpp
Widget w;
auto&& w1 = w;               // w1 is an lvalue reference

auto&& w2 = widgetFactory(); // w2 is an rvalue reference
```

A universal reference isn't a new kind of reference, it's actually an rvalue reference in a context where two conditions are satisfied:
* type deduction distinguishes lvalues from rvalues. Lvalues of type T are deduced to have type T&, while rvalues of type T yield T as their deducted type.
* reference collapsing occurs.
The concept of universal reference is helpful to free you from recognizing the existence of reference collapsing contexts.

Typedef and using alias declarations also employ reference collapsing.
```cpp
template<typename T>
class Widget {
public:
  typedef T&& RvalueRefToT;
  …
};

// suppose we have
Widget<int&> w;
// reference collapse makes it
typedef int& RvalueRefToT;
```

decltype also employs reference collapsing during its type analysis.

**Takeaways**
* Reference collapsing occurs in four contexts: template instantiation, auto type generation, creation and use of typedefs and alias declarations, and decltype.
* When compilers generate a reference to a reference in a reference collapsing context, the result becomes a single reference. If either of the original references is an lvalue reference, the result is an lvalue reference. Otherwise it’s an rvalue reference.
* Universal references are rvalue references in contexts where type deduction distinguishes lvalues from rvalues and where reference collapsing occurs.

### Assume that move operations are not present, not cheap, and not used

True that move may offer efficiency gains, but working with C++98 codebase you've no reason to assume move will be present (whose generation, will be suppressed if any custom copy, move, or dtor is in place, it17.)

All containers in the standard C++11 library support moving, but moving in these containers may not be cheap.
Consider std::array added in C++11, a built-in array with an STL interface.
std::array is fundamentally different the other standard containers, each of which stores its contents on the heap.
Objects of those container types hold conceptually pointers to the heap location storing contents of the container, making their move constant time. E.g.

```cpp
std::vector<Widget> vw1;

// put data into vw1

…

// move vw1 into vw2. Runs in
// constant time. Only ptrs
// in vw1 and vw2 are modified
auto vw2 = std::move(vw1);
```

Such constant time move is not the case for std::array, whose content may not live in heap and have a pointer to the heap location.

```cpp
std::array<Widget, 10000> aw1;

// put data into aw1

…


// move aw1 into aw2. Runs in
// linear time. All elements in
// aw1 are moved into aw2
auto aw2 = std::move(aw1);
```

On the other hand, std::string offers constant-time moves and linear-time copies.
This makes it sound like moving is faster than copying, but that may not be the case.

Many string impl use small string optimization, in which small strings (<15 characters) are stored in a buffer within the std::string object as opposed to on the heap.
With SSO moving a string incurs the same cost as copying them (can't employ move-only-a-pointer trick)

Even with containers supporting move, some move situations may actually incur copying.
It14 explains due to strong exception guarantee, some can only move if they know the underlying move is noexcept.

Thus with C++11 you can still end up with no move operations, move not faster than copy, move not usable, or the source object is lvalue (with few exceptions only rvalues are eligible as sources of move, it25).

**Takeaways**
* Assume that move operations are not present, not cheap, and not used.
* In code with known types or support for move semantics, there is no need for assumptions.

### Familiarize yourself with perfect forwarding failure cases

What we mean by perfect forwarding: one function passes (forwards) its parameters to another function. (the second function should receive the first parameters that the first function receives, which rules out pass-by-value, since they are copies of what the original caller passes in)
When it comes to general purpose forwarding, we are dealing with parameters that are references.



