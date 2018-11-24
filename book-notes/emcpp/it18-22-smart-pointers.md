# Smart Pointers

Why raw pointers are hard to love:
* its declaration doesn't indicate it points to a single object or an array
* its declaration reveals nothing about whether you should destroy the thing it's pointing to, when you are done using it: no ownership indication
* if you are to delete it, there's no indication if you should use a `delete` call, or a different mechanism (passing the pointer to some deletion function)
* if you do `delete`, do you `delete` or `delete[]`
* assuming you figure out how to delete it, it's difficult to ensure that you perform destruction exactly once along every path in your code
* there's typically no way to tell if the pointer dangles (point to memory that no longer holds the object)

Smart pointers do virtually everything raw pointers can, but with far fewer opportunity for error.

C++11 has `std::auto_ptr`, `std::unique_ptr`, `std::shared_ptr`, `std::weak_ptr`.
`std::auto_ptr` was in C++98 and deprecated in C++11.
Doing `unique_ptr` right requires move semantics.
As a workaround `std::auto_ptr` co-opted its copy opr for moves.
Unless you have the constraint of using C++98, use `std::unique_ptr` and never look back.

### Item 18: use `std::unique_ptr` for exclusive-ownership resource management

It's reasonable to assume that by default `std::unique_ptr` is the same size as raw pointers.
If a raw pointer is small enough and fast enough for you, a `std::unique_ptr` almost certainly is, too.

`std::unique_ptr` embodies exclusive ownership semantics.
A non-null `std::unique_ptr` always owns what it points to.

`std::unqiue_ptr` is move-only. Copy is not allowed.
Moving `std::unique_ptr` transfers ownership from source pointer to destination pointer.

Upon destruction, a non-null `std::unique_ptr` destroys its resource.
By default resource destruction is accomplished by applying `delete` to the raw pointer inside the `std::unique_ptr`, but during construction `std::unique_ptr` objects can be configured to take custom deleters: arbitrary function to invoke when deletion happens.

A common use case for `std::unique_ptr` is as a factory function return type for objects in a hierarchy.
Factory constructs an object on the heap, and hands ownership to the user.

A factory method example with custom deleter:
```cpp
auto delInvmt = [](Investment* pInvestment)       // custom
                {                                 // deleter
                  makeLogEntry(pInvestment);      // (a lambda
                  delete pInvestment;             // expression)
                };
// delete the child object via pointer to parent. Parent must
// have virtual dtor.

template<typename... Ts>                          // revised
std::unique_ptr<Investment, decltype(delInvmt)>   // return type
// can return auto in C++14
makeInvestment(Ts&&... params)
{
  std::unique_ptr<Investment, decltype(delInvmt)> // ptr to be
    pInv(nullptr, delInvmt);                      // returned

  if ( /* a Stock object should be created */ ) {
    pInv.reset(new Stock(std::forward<Ts>(params)...));
  } else if ( /* a Bond object should be created */ ) {
    pInv.reset(new Bond(std::forward<Ts>(params)...));
  } else if ( /* a RealEstate object should be created */ ) {
    pInv.reset(new RealEstate(std::forward<Ts>(params)...));
  }

  return pInv;
}
```

Custom deleters generally cause the size the a `std::unique_ptr` to grow from one word to two.

Stateless function objects (e.g., lambda expressions with no captures) typically incur no size penalty when used as deleters, and this means that when a custom deleter can be implemented as either a function or a captureless lambda expression, the lambda is preferable.

Function object deleters with extensive state can yield `std::unique_ptr` objects of significant size. (because the state would then need to be associated with each instance of the pointer object)

`std::unique_ptr` is often used to implement pimpl idiom.

The existence of `std::unique_ptr` for arrays should be of only intellectual interest to you, because `std::array`, `std::vector`, and `std::string` are virtually always better data structure choices than raw arrays.
About the only situation I can conceive of when a `std::unique_ptr<T[]>` would make sense would be when you're using a C-like API that returns a raw pointer to a heap array that you assume ownership of.

`std::unique_ptr` is the C++11 way to express exclusive ownership, but one of its most attractive features is that it easily and efficiently converts to a `std::shared_ptr`.

This is a key part of why `std::unique_ptr` is so well suited as a factory function return type.
Factory functions can’t know whether callers will want to use exclusive-ownership semantics for the object they return or whether shared ownership (i.e., `std::shared_ptr`) would be more appropriate.
By returning a `std::unique_ptr`, factories provide callers with the most efficient smart pointer, but they don't hinder callers from replacing it with its more flexible sibling.

_How is `bslma::ManagedPtr` in C++03 without move semantics?_

From a rough look, `bslma::ManagedPtr` does support the unnatural `auto_ptr` copycon, which takes in modifiable reference and transfers ownership in a "copy constructor".
However, it also has a ctor taking in `bslmf::MovableRef`, which seems to BDE's backport of move semantics and requires more research.

**Takeaways**
* `std::unique_ptr` is a small, fast, move-only smart pointer for managing resources with exclusive-ownership semantics.
* By default, resource destruction takes place via `delete`, but custom deleters can be specified. Stateful deleters and function pointers as deleters increase the size of `std::unique_ptr` objects.
* Converting a `std::unique_ptr` to a `std::shared_ptr` is easy.

### Item 19: use `std::shared_ptr` for shared-ownership resource management

An argument for manually manage memory (and dtors) may be the deterministic nature and predictability of when resource reclamation is going to happen.

Why can't we have the best of both worlds:
a system that works automatically (like garbage collection), yet applies to all resources and has predictable timing (like destructors)?
C++11 `std::shared_ptr` does this.

An object accessed via `std::shared_ptr` has its lifetime managed by these shared pointers.
No single one assumes ownership, they collaborate to make sure the object is destroyed when it's no longer needed (last `shared_ptr` stops pointing at the object).

A shared pointer knows how many others are pointing at the object by consulting the resource's reference count.
(Usually ctor increments it (move ctor doesn't), dtor decrements it, copy assignment opr does both.)
When after a decrement the count drops to 0, the `shared_ptr` destroys the object.

As a result
* `std::shared_ptr` is twice the size of unique pointers: it holds a reference to the object and another reference to the reference count of the object.
* Memory for the reference count needs to be dynamically allocated. (Pointed to object has no idea it's managed by a pointer)
* Increments and decrements to reference count needs to be atomic to guarantee threadsafety. Atomic operations are typically slower than the non-atomic counterparts.

Move assignment / ctor is faster than copy assignment / ctor for `std::shared_ptr`, since move doesn't involve atomic increments / decrements but copy does.

Note that if a custom deleter is given it's not part of `shared_ptr`'s type.
This is not the case with `unique_ptr`.
```cpp
auto loggingDel = [](Widget *pw)        // custom deleter
                  {                     // (as in Item 18)
                    makeLogEntry(pw);
                    delete pw;
                  };

std::unique_ptr<                        // deleter type is
  Widget, decltype(loggingDel)          // part of ptr type
  > upw(new Widget, loggingDel);

std::shared_ptr<Widget>                 // deleter type is not
  spw(new Widget, loggingDel);          // part of ptr type
```
The `shared_ptr` deleter design is more flexible.
Having a custom deleter changes the size of the `unique_ptr` (and that deleter is part of `unique_ptr`'s type), while having a custom deleter does not change the size of a `shared_ptr`.

Why the difference? `shared_ptr` stores the deleter not inside each shared pointer object, but together with the reference count, as part of a "control block".

The control block contains reference count, a custom deleter if specified, a custom allocator if specified, and a secondary reference count "the weak count".

The following rules exist for creating control blocks:
* `std::make_shared` always creates a control block
* when a `shared_ptr` is created from `unique_ptr` or `auto_ptr` (and as part of the construction the `unique_ptr` is set to null)
* when a `shared_ptr` is called with a raw pointer
* `shared_ptr` created from `shared_ptr` or `weak_ptr` don't allocate new control blocks. They expect the control block to be passed in.

As a consequence, more than one `shared_ptr`s created from a raw pointer means more than one control blocks thus double free and UB.
Avoid doing this
```cpp
auto pw = new Widget;                          // pw is raw ptr

…

std::shared_ptr<Widget> spw1(pw, loggingDel);  // create control
                                               // block for *pw
…

std::shared_ptr<Widget> spw2(pw, loggingDel);  // create 2nd
                                               // control block
                                               // for *pw!
```

Two lessons
* avoid passing raw pointers to `std::shared_ptr`. Use `make_shared` instead.
* if you have to pass a raw pointer to a `shared_ptr`, pass the result directly from new instead.

A particular case to be careful about is `this` pointer.
Say we have the following vector to keep track of processed widgets.
```cpp
std::vector<std::shared_ptr<Widget>> processedWidgets;
...
class Widget {
public:
  ...
  void process() {
      processedWidgets.emplace_back(this);    // add it to list of
                                              // processed Widgets;
                                              // this is wrong!
  }
  ...
};
// if there can be other shared pointers to this object, the code is going to UB.
```

You could do instead
```cpp
class Widget: public std::enable_shared_from_this<Widget> {
public:
  ...
  void process() {
    processedWidgets.emplace_back(shared_from_this()); // fine
  }
  ...
};
// Widget derives from std::enable_shared_from_this with Widget itself as a template
// argument. This is completely legal and has a name "Curiously Recurring Template Pattern".
```

`std::enable_shared_from_this` defines a function `shared_from_this()` that allocates control block to the current object without duplicating. Use `shared_from_this()` when you want a `shared_ptr` that points to the same object as `this`.

Underlying `std::enable_shared_from_this` it relies on the current object having a control block, and there must be an existing `shared_ptr` (outside the member function calling `shared_from_this`) pointing to this.
If not, `shared_from_this` typically throws.

To make sure such a `shared_ptr` exists, classes deriving from `std::enable_shared_from_this` typically hides ctor and provides a factory method returning `shared_ptr`. E.g.
```cpp
class Widget: public std::enable_shared_from_this<Widget> {
public:
  // factory function that perfect-forwards args
  // to a private ctor
  template<typename... Ts>
  static std::shared_ptr<Widget> create(Ts&&... params);

  ...
  void process();             // as before
  ...

private:
  …                           // ctors
};
```

Control blocks come at a cost, they may have arbitrarily large deleters, and the underlying impl uses inheritance so there's also vptr (_how?_).

But for the functionality they provide, `shared_ptr`'s cost is very reasonable.
With default deleter and allocator, and created with `make_shared`, the control block is 3 words in size, and its allocation is essentially free. Dereferencing is cheap, atomic operations should map to machine instructions.
If you want to model shared ownership, `shared_ptr` is still the right way to go.

`unique_ptr` cannot be created from `shared_ptr`.

Another thing `shared_ptr` can't do is working with arrays, no array template parameters, unlike `unique_ptr`.
But given different alternatives to built-in array (e.g. `array`, `vector`, `string`), using a smart pointer to manage a dumb array is probably a bad idea in the first place.

**Takeaway**
* `std::shared_ptr`s offer convenience approaching that of garbage collection for the shared lifetime management of arbitrary resources.
* Compared to `std::unique_ptr`, `std::shared_ptr` objects are typically twice as big, incur overhead for control blocks, and require atomic reference count manipulations.
* Default resource destruction is via delete, but custom deleters are supported. The type of the deleter has no effect on the type of the `std::shared_ptr`.
* Avoid creating `std::shared_ptr`s from variables of raw pointer type.

### Use std::weak\_ptr for std::shared\_ptr like pointers that can dangle

A weak\_ptr is like a shared\_ptr that does not affect an object's reference count.
Thus they face the possibility of the object being destroyed when they try to access it.

A weak\_ptr cannot be dereferenced directly, nor can they be tested for nullness.
It's because it isn't standalone pointer but rather an augmentation of shared\_ptr.

The relationship begins at birth. std::weak\_ptr is typically created from std::shared\_ptr.
They point to the same place as std::shared\_ptr, but they don't affect the reference count in the control block.

Weak pointers that dangle are said to have expired.

```cpp
auto spw =                       // after spw is constructed,
  std::make_shared<Widget>();    // the pointed-to Widget's
                                 // ref count (RC) is 1. (See
                                 // Item 21 for info on
                                 // std::make_shared.)
…

std::weak_ptr<Widget> wpw(spw);  // wpw points to same Widget
                                 // as spw. RC remains 1
…
if (wpw.expired()) …             // if wpw doesn't point
                                 // to an object…
```

Often you want to do check if expired, if not, dereference.
But if you do it in two steps, a race condition would be introduced.
Thus you need one atomic operation of check if expired, if not, create a shared\_ptr from it. This is called lock. Shared pointer ctor taking in a weak pointer is the same operation as lock, just that it throws if the weak\_ptr has expired.

```cpp
// Form 1 of lock
std::shared_ptr<Widget> spw1 = wpw.lock();  // if wpw's expired,
                                            // spw1 is null

auto spw2 = wpw.lock();                     // same as above,
                                            // but uses auto

// Form 2 of lock
std::shared_ptr<Widget> spw3(wpw);    // if wpw's expired,
                                      // throw std::bad_weak_ptr
```

How are weak pointers useful?

One case is the following: imagine you have a loadWidget(id) call, which by itself is expensive and you want to cache things by id inside.
You can't have an unlimited cache.
One way to implement it is use a weak\_ptr inside: cache the Widgets inside with weak pointers, give loaded objects back to the client, let client manage their shared ownership. When another load is called, cache trys locking and if it hasn't expired, cache can just serve the content.
```cpp
std::shared_ptr<const Widget> fastLoadWidget(WidgetID id)
{
  static std::unordered_map<WidgetID,
                            std::weak_ptr<const Widget>> cache;

  auto objPtr = cache[id].lock();   // objPtr is std::shared_ptr
                                    // to cached object (or null
                                    // if object's not in cache)

  if (!objPtr) {                    // if not in cache,
    objPtr = loadWidget(id);        // load it
    cache[id] = objPtr;             // cache it
  }
  return objPtr;
}
```

Another use case is the observer pattern, in which there is subjects (objects whose state may change) and observers (objects to be notified when a state change happens).
Subjects typically hold a pointer to observers, so that they can be notified when a state change happens.
Subjects have no interest in the lifetime of observers, but they care if an observer is destroyed they don't make subsequent access to it.
A reasonable design is to let subjects hold weak pointers to observers.

A third use case is to break cycles in cycling reference by shared\_ptr. Instead of A and B holding shared\_ptrs to each other, A to B could be shared\_ptr and B to A could be weak\_ptr.
It's worth noting that this should be a rare case: in a typical parent's lifetime outlives that of its children's use case, parent could hold unique\_ptr to children and children could hold a raw pointer back to parent if needed.

From an efficiency perspective, weak\_ptr makes the same case as shared\_ptr: same size, control block, and operations such as construction, destruction and assignment involves atomic reference count manipulations (of weak count in control block. _Why do we need weak count?_).

**Takeaways**
* Use std::weak_ptr for std::shared_ptr-like pointers that can dangle.
* Potential use cases for std::weak_ptr include caching, observer lists, and the prevention of std::shared_ptr cycles.

### Prefer std::make\_unique and std::make\_shared to direct use of new

std::make\_unique is part of C++14.
std::make\_shared is part of C++11. If you need to implement it yourself, it looks like (without supporting arrays)
```cpp
template<typename T, typename... Ts>
std::unique_ptr<T> make_unique(Ts&&... params)
{
  return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}
```
std::allocate\_shared acts like make\_shared, except its first argument is an allocator to be used for the underlying dynamic memory allocation.

Compare the version using make\_xxx and using raw pointer ctor:
```cpp
auto upw1(std::make_unique<Widget>());      // with make func

std::unique_ptr<Widget> upw2(new Widget);   // without make func


auto spw1(std::make_shared<Widget>());      // with make func

std::shared_ptr<Widget> spw2(new Widget);   // without make func
```
The version without repeats the type Widget, whose duplication should be avoided.

Another concern is exception safety.
Consider this code where we process widget with a priority (pass shared\_ptr by value looks suspicious, how?):
```cpp
void processWidget(std::shared_ptr<Widget> spw, int priority);

int computePriority();

// call site
processWidget(std::shared_ptr<Widget>(new Widget),  // potential
              computePriority());                   // resource
                                                    // leak!
```

Why potential resource leak?
Three things need to happen here, new Widget, shared\_ptr ctor, computePriority() call.
Compiler is allowed to generate code that put computePriority call in between.
Thus if computePriority call throws, heap allocation of Widget is done, but the memory won't be managed by a smart pointer.
std::make\_shared avoids such a problem.
```cpp
processWidget(std::make_shared<Widget>(),   // no potential
              computePriority());           // resource leak
// even though either one of make_shared and computePriority can be called first in the compiler generated code 
```

std::make\_shared also improves efficiency.
```cpp
std::shared_ptr<Widget> spw(new Widget);
// two memory allocations of Widget object and its control block
auto spw = std::make_shared<Widget>();
// one memory allocation to hold both the object and the control block.
```

There are circumstances where make\_shared and make\_unique cannot be used.
First is when deleter is passed in as argument.
Second is this: make\_unique and make\_shared perfectly forwards to the object's ctor, but do they forwward using parentheses or brackets?
```cpp
auto upv = std::make_unique<std::vector<int>>(10, 20);

auto spv = std::make_shared<std::vector<int>>(10, 20);
// Forwards using () or {} makes a difference for vectors!
```
They use parentheses, which means if you want to initialize the object using brackets, you can either use new, or pass an std::initializer\_list:
```cpp
// create std::initializer_list
auto initList = { 10, 20 };

// create std::vector using std::initializer_list ctor
auto spv = std::make_shared<std::vector<int>>(initList);
```

For make\_shared, there are two more caveats.
* Classes with custom operator new and delete, who typically allocates the exact size of the Object in their new.
std::allocate\_shared need to request size of Object + size of control block, it usually doesn't work well with overloaded new.
* Big objects that has shared\_ptrs and weak\_ptrs pointing to it, and wants the object to be destroyed when all shared\_ptr references are gone.
Since make\_shared allocates the control block and the object together, and the control block is only freed after all shared\_ptr as well as weak\_ptr references are gone, make\_shared created objects will not have the freedom as new'ed objects do to deallocate the object and the control block separately.

If you have to use new, watch out for the exception-safety issue mentioned earlier.

You could do
```cpp
std::shared_ptr<Widget> spw(new Widget, cusDel);

processWidget(spw, computePriority());     // correct, but not
                                           // optimal: pass shared_ptr by
                                           // lvalue incurs a copy thus
                                           // additional atomic opr 
// Or instead
processWidget(std::move(spw),            // both efficient and
              computePriority());        // exception safe
```

**Takeaways**
* Compared to direct use of new, make functions eliminate source code duplication, improve exception safety, and, for std::make_shared and std::allocate_shared, generate code that’s smaller and faster.
* Situations where use of make functions is inappropriate include the need to specify custom deleters and a desire to pass braced initializers.
* For std::shared_ptrs, additional situations where make functions may be ill-advised include (1) classes with custom memory management and (2) systems with memory concerns, very large objects, and std::weak_ptrs that outlive the corresponding std::shared_ptrs.

### When using the pimpl idiom, define special member functions in the implementation file

Pimpl idiom is often used to combat excessive build times.
Why does it help? Consider the following
```cpp
class Widget {                     // in header "widget.h"
public:
  Widget();
  …
private:
  std::string name;
  std::vector<double> data;
  Gadget g1, g2, g3;               // Gadget is some user-
};                                 // defined type
```
This means the header has to include vector, string, and gadget.
These headers in turn increase the build time of Widget's clients, and if header contents (e.g. gadget and widget) change, the client has to recompile.
With pimpl you have
```cpp
class Widget {                 // still in header "widget.h"
public:
  Widget();
  ~Widget();                   // dtor is needed—see below
  …

private:
  struct Impl;                 // declare implementation struct
  Impl *pImpl;                 // and pointer to it
};
```
Where Widget::Impl is an incomplete type.
There is little you can do an incomplete type, but you can make a pointer to it, given the size is known.
The impl in C++98 then looks something like this:
```cpp
#include "widget.h"            // in impl. file "widget.cpp"
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl {          // definition of Widget::Impl
  std::string name;            // with data members formerly
  std::vector<double> data;    // in Widget
  Gadget g1, g2, g3;
};

Widget::Widget()               // allocate data members for
: pImpl(new Impl)              // this Widget object
{}

Widget::~Widget()              // destroy data members for
{ delete pImpl; }              // this object
```

With C++11, unqiue\_ptr is exactly the tool we need, and the code in turn looks something like this
```cpp
#include "widget.h"                 // in "widget.cpp"
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl {               // as before
  std::string name;
  std::vector<double> data;
  Gadget g1, g2, g3;
};

Widget::Widget()                    // per Item 21, create
: pImpl(std::make_unique<Impl>())   // std::unique_ptr
{}                                  // via std::make_unique
```

However when we use it in a different translation unit, 
```cpp
Widget w;
// Compiler generates: invalid application of 'sizeof' to an incomplete type 'Widget::Impl'
```
Problem is that as compiler generates code for w's deletion (delete on the raw pointer inside unique_ptr), delete needs to be called on a complete type.
In this translation unit with pimpl.h included, struct Impl is not a complete type.

So we declare Widget's dtor in the header but not define it.
```cpp
class Widget {                     // as before, in "widget.h"
public:
  Widget();
  ~Widget();                       // declaration only
  …

private:                           // as before
  struct Impl;
  std::unique_ptr<Impl> pImpl;
};

// And
#include "widget.h"                // as before, in "widget.cpp"
#include "gadget.h"
#include <string>
#include <vector>

struct Widget::Impl {              // as before, definition of
  std::string name;                // Widget::Impl
  std::vector<double> data;
  Gadget g1, g2, g3;
};

Widget::Widget()                   // as before
: pImpl(std::make_unique<Impl>())
{}

Widget::~Widget()                  // ~Widget definition
{}
// alternative dtor
Widget::~Widget() = default;       // same effect as above
```

Pimpls are often times great candidates for move. So we add in the move operations.
```cpp
// header
...
  Widget(Widget&& rhs) noexcept;              // declarations
  Widget& operator=(Widget&& rhs) noexcept;   // only
...

// impl
Widget::Widget(Widget&& rhs) noexcept = default;              
Widget& Widget::operator=(Widget&& rhs) noexcept = default;

// you can't do "= default" in the header, since that would make it a definition and
// to be able to generate code for the default move (move assignment needs to destroy
// the previously managed item; move ctor needs to be able to delete Impl in case of an
// exception, even though you declare it noexcept), Impl needs to be a complete type
```

And we'll need to write the copy operations ourselves since compiler won't be generate copy operations for classes with move-only types like unique\_ptr. (Even if they do it'd be a shallow copy of the pointer not a deeo copy of the underlying object)

```cpp
// deep copy of an object using pimpl
Widget::Widget(const Widget& rhs)              // copy ctor
: pImpl(nullptr)
{ if (rhs.pImpl) pImpl = std::make_unique<Impl>(*rhs.pImpl); }

Widget& Widget::operator=(const Widget& rhs)   // copy operator=
{
  if (!rhs.pImpl) pImpl.reset();
  else if (!pImpl) pImpl = std::make_unique<Impl>(*rhs.pImpl);
  else *pImpl = *rhs.pImpl;

  return *this;
}
```

Yet if we use a shared\_ptr for pimpl, the rules of this chapter don't apply.
This would work just fine in client code and the compiler will supply the big five.
```cpp
class Widget {                     // in "widget.h"
public:
  Widget();
  …                                // no declarations for dtor
                                   // or move operations
private:
  struct Impl; 
  std::shared_ptr<Impl> pImpl;     // std::shared_ptr
};                                 // instead of std::unique_ptr
```

The difference between shared\_ptr and unique\_ptr stems from custom deleter support: in unique\_ptr due to deleter being part of the type (allowing smaller runtime structures and faster runtime code), the type must be complete when using compiler generated dtor or moves.
Such restriction is lifted in shared\_ptr's case with deleter not being part of the type.

To use unique\_ptr or shared\_ptr depends on the use case.
It's possible pimpls could desire shared ownership of the underlying.

**Takeaways**
* The Pimpl Idiom decreases build times by reducing compilation dependencies between class clients and class implementations.
* For std::unique_ptr pImpl pointers, declare special member functions in the class header, but implement them in the implementation file. Do this even if the default function implementations are acceptable.
* The above advice applies to std::unique_ptr, but not to std::shared_ptr.

(_think about it, even though my custom deleter and compiler supplied deleter does the same thing, we still can't use compiler's in unique\_ptr's case. Is it because the inline-by-default nature of compiler generated ones?_)
