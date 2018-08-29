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

