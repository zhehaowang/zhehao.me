# Smart Pointers

Why raw pointers are hard to love:
* its declaration doesn't indicate it points to a single object or an array
* its declaration reveals nothing about whether you should destroy the thing it's pointing to, when you are done using it: no ownership indication
* if you are to delete it, there's no indication if you should use a delete call, or a different mechanism (passing the pointer to some deletion function)
* if you do delete, do you delete or delete[]
* assuming you figure out how to delete it, it's difficult to ensure that you perform destruction exactly once along every path in your code
* there's typically no way to tell if the pointer dangles (point to memory that no longer holds the object)

Smart pointers do virtually everything raw pointers can, but with far fewer opportunity for error.

C++11 has std::auto\_ptr, std::unique\_ptr, std::shared\_ptr, std::weak\_ptr.
std::auto\_ptr was in C++98 and deprecated in C++11. Doing unique\_ptr right requires move semantics.
As a workaround std::auto\_ptr co-opted its copy opr for moves.
Unless you have the constraint of using C++98, use std::unique\_ptr and never look back.

### Use std::unique\_ptr for exclusive-ownership resource management

It's reasonable to assume that by default std::unique\_ptr is the same size as raw pointers.
If a raw pointer is small enough and fast enough for you, a std::unique\_ptr almost certainly is, too.

std::unique\_ptr embodies exclusive ownership semantics.
A non-null std::unique\_ptr always owns what it points to.

std::unqiue\_ptr is move-only. Copy is not allowed.
Moving std::unique\_ptr transfers ownership from source pointer to destination pointer.

Upon destruction a non-null std::unique\_ptr destroys its resource.
By default resource destruction is accomplished by applying delete to the raw pointer inside the std::unique\_ptr, but during construction std::unique\_ptr objects can be configured to take custom deleters: arbitrary function to invoke when deletion happens.

A common use case for std::unique\_ptr is as a factory function return type for objects in a hierarchy.
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

  if ( /* a Stock object should be created */ )
  {
    pInv.reset(new Stock(std::forward<Ts>(params)...));
  }
  else if ( /* a Bond object should be created */ )
  {
    pInv.reset(new Bond(std::forward<Ts>(params)...));
  }
  else if ( /* a RealEstate object should be created */ )
  {
    pInv.reset(new RealEstate(std::forward<Ts>(params)...));
  }

  return pInv;
}
```

Custom deleters generally cause the size the a std::unique\_ptr to grow from one word to two.
Stateless function objects (e.g., from lambda expressions with no captures) typically incur no size penalty when used as deleters, and this means that when a custom deleter can be implemented as either a function or a captureless lambda expression, the lambda is preferable.
Function object deleters with extensive state can yield std::unique_ptr objects of significant size. (_why?_)

std::unique\_ptr is often used to implement pimpl idiom.

The existence of std::unique\_ptr for arrays should be of only intellectual interest to you, because std::array, std::vector, and std::string are virtually always better data structure choices than raw arrays.
About the only situation I can conceive of when a std::unique\_ptr<T[]> would make sense would be when you’re using a C-like API that returns a raw pointer to a heap array that you assume ownership of.

std::unique\_ptr is the C++11 way to express exclusive ownership, but one of its most attractive features is that it easily and efficiently converts to a std::shared\_ptr.
This is a key part of why std::unique_ptr is so well suited as a factory function return type. Factory functions can’t know whether callers will want to use exclusive-ownership semantics for the object they return or whether shared ownership (i.e., std::shared\_ptr) would be more appropriate. By returning a std::unique\_ptr, factories provide callers with the most efficient smart pointer, but they don’t hinder callers from replacing it with its more flexible sibling.

How is bslma::ManagedPtr in C++03 without move semantics?
From a rough look, bslma::ManagedPtr does support the unnatural auto\_ptr copycon, which takes in modifiable reference and transfers ownership in a "copy constructor".
However, it also has a ctor taking in bslmf::MovableRef, which seems to BDE's backport of move semantics and requires more research.

**Takeaways**
* std::unique_ptr is a small, fast, move-only smart pointer for managing resources with exclusive-ownership semantics.
* By default, resource destruction takes place via delete, but custom deleters can be specified. Stateful deleters and function pointers as deleters increase the size of std::unique_ptr objects.
* Converting a std::unique_ptr to a std::shared_ptr is easy.

### Use std::shared\_ptr for shared-ownership resource management

An argument for manually manage memory (and dtors) may be the deterministic nature and predictability of when resource reclamation is going to happen.
Why can’t we have the best of both worlds: a system that works automatically (like garbage collection), yet applies to all resources and has predictable timing (like destructors)? C++11 std::shared\_ptr does this.
An object accessed via std::shared\_ptr has its lifetime managed by these shared pointers. No single one assumes ownership, they collaborate to make sure the object is destroyed when it's no longer needed (last shared\_ptr stops pointing at the object).

A shared pointer knows how many others are pointing at the object by consulting the resource's reference count. (Usually ctor increments it (move ctor doesn't), dtor decrements it, copy assignment opr does both.)
When after a decrement the count drops to 0, the shared\_ptr destroys the object.

As a result
* std::shared\_ptr is twice the size of unique pointers: it holds a reference to the object and another reference to the reference count of the object.
* Memory for the reference count needs to be dynamically allocated. (Pointed to object has no idea it's managed by a pointer)
* Increments and decrements to reference count needs to be atomic to guarantee threadsafety. Atomic operations are typically slower than the non-atomic counterparts.

Move assignment / ctor is faster than copy assignment / ctor for std::shared\_ptr, since move doesn't involve atomic increments / decrements but copy does.

Note that if a custom deleter is given it's not part of shared\_ptr's type. This is not the case the unique\_ptr.
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
The shared\_ptr design is more flexible.
Having a custom deleter changes the size of the unique\_ptr, while having a custom deleter does not change the size of a shared\_ptr.

Why the difference? shared\_ptr stores the deleter not inside each shared pointer object, but together with the reference count, as part of a "control block".

The control block contains reference count, a custom deleter if specified, a custom allocator if specified, and a secondary reference count "the weak count".

The following rules exist for creating control blocks:
* std::make\_shared always creates a control block
* when a shared\_ptr is created from unique\_ptr or auto\_ptr (and as part of the construction the unique\_ptr is set to null)
* when a shared\_ptr is called with a raw pointer
* shared\_ptr created from shared\_ptr or weak\_ptr don't allocate new control blocks. They expect the control block to be passed in.

As a consequence, more than one shared\_ptrs created from a raw pointer means more than one control blocks thus double free and UB.
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
* avoid passing raw pointers to std::shared\_ptr. Use make\_shared instead.
* if you have to pass a raw pointer to a shared\_ptr, pass the result directly from new instead.

A particular case to be careful about is this pointer.
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
  …
  void process() {
    processedWidgets.emplace_back(shared_from_this()); // fine
  }
  …
};
// Widget derives from std::enable_shared_from_this with Widget itself as a template
// argument. This is completely legal and has a name "Curiously Recurring Template Pattern".
```

std::enable\_shared\_from\_this defines a function shared\_from\_this() that allocates control block to the current object without duplicating. Use shared\_from\_this() when you want a shared\_ptr that points to the same object as this.

Underlying std::enable\_shared\_from\_this it relies on the current object having a control block, and there must be an existing shared\_ptr (outside the member function calling shared\_from\_this) pointing to this. If not, shared\_from\_this typically throws.
To make sure such a shared\_ptr exists, classes deriving from std::enable\_shared\_from\_this typically hides ctor and provides a factory method returning shared\_ptr. E.g.
```cpp
class Widget: public std::enable_shared_from_this<Widget> {
public:
  // factory function that perfect-forwards args
  // to a private ctor
  template<typename... Ts>
  static std::shared_ptr<Widget> create(Ts&&... params);

  …
  void process();             // as before
  …

private:
  …                           // ctors
};
```

Control blocks come at a cost, they may have arbitrarily large deleters, and the underlying impl uses inheritance so there's also vptr.
But for the functionality they provide, shared\_ptr's cost is very reasonable.
With default deleter and allocator, and created with make\_shared, the control block is 3 words in size, and its allocation is essentially free. Dereferencing is cheap, atomic operations should map to machine instructions.
If you want to model shared ownership, shared\_ptr is still the right way to go.

unique\_ptr cannot be created from shared\_ptr.

Another thing shared pointers can't do is working with arrays, no array template parameters, unlike unique\_ptr.
But given different alternatives to built-in array (e.g. array, vector, string), using a smart pointer to manage a dumb array is probably a bad idea in the first place.

**Takeaway**
* std::shared\_ptrs offer convenience approaching that of garbage collection for the shared lifetime management of arbitrary resources.
* Compared to std::unique\_ptr, std::shared\_ptr objects are typically twice as big, incur overhead for control blocks, and require atomic reference count manipulations.
* Default resource destruction is via delete, but custom deleters are supported. The type of the deleter has no effect on the type of the std::shared\_ptr.
* Avoid creating std::shared\_ptrs from variables of raw pointer type.

### Use std::weak\_ptr for std::shared\_ptr like pointers that can dangle



