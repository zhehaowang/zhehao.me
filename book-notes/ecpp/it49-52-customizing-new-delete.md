# Customizing `new` and `delete`

This chapter discusses heap memory management, with heap being a shared resource across threads.
Be on high alert when applying these in threaded systems.

Something else to be aware: `operator new` and `operator delete` allocate / deallocate single objects.
Arrays are allocated and deallocated by `operator new[]` and `operator delete[]`.

Finally, know that STL container's heap allocation is done by the container's allocator, not `new` `delete` operators.

### Understand the behavior of the `new` handler

When `operator new` cannot satisfy a memory allocation request, it throws an exception.
Before throwing an exception, `new` calls a client specifiable new-handler.
Client can specify this handler via `set_new_handler` in std.
```cpp
#include <new>

namespace std {
  typedef void (*new_handler)();
  new_handler set_new_handler(new_handler p) throw();
}
```
A `new` handler takes nothing returns nothing, and `set_new_handler` takes a handler and returns a handler (a pointer to the handler function in effect).
The `throw()` at the end suggests this function throws no exceptions.

Use `set_new_handler` like this
```
// function to call if operator new can't allocate enough memory
void outOfMem() {
  std::cerr << "Unable to satisfy request for memory\n";
  std::abort();
}

int main() {
  std::set_new_handler(outOfMem);
  int *pBigDataArray = new int[100000000L];
  ...
}
```
When `operator new` cannot find enough memory, it calls `new` handler until it can find enough memory.
So a well designed `new` handler should do the following
* Make more memory available. (E.g. allocating a chunk at the program start, and release when `new` handler is called)
* Install a different `new` handler. If the current handler doesn't know how to get more memory, maybe it knows a different handler that can.
* Deinstall the `new` handler. (i.e. pass `null` pointer to `set_new_handler`). With no `new` handler installed, next time when memory allocation is not successful `operator new` throws.
* Throw exception. Of `bad_alloc` or something derived from `bad_alloc`. Such exceptions will not be captured by `operator new` and will propagate to the site originating the memory request.
* Not return. Typically `abort` or `exit`.

Sometimes you want different classes to have different behaviors when running out of memory.
```cpp
class X {
public:
  static void outOfMemory();
  ...
};
class Y {
public:
  static void outOfMemory();
  ...
};
X* p1 = new X;                        // if allocation is unsuccessful,
                                      // call X::outOfMemory

Y* p2 = new Y;                        // if allocation is unsuccessful,
                                      // call Y::outOfMemory
```
You can achieve this behavior by having each class provide their own `set_new_handler` and `operator new`, where the class's `set_new_handler` allows clients to specify `new` handlers for the class, and the class's custom `operator new` ensures the class-specific handler is called instead of the global one.

It'll look something like this
```cpp
class Widget {
public:
  static std::new_handler set_new_handler(std::new_handler p) throw();
  static void * operator new(std::size_t size) throw(std::bad_alloc);
private:
  static std::new_handler currentHandler;
};

std::new_handler Widget::currentHandler = 0;    // init to null in the class
                                                // impl. file
// static class members must be defined outside the class definition
// (unless they're const and integral—see Item 2)

// save and return the old, set the new
std::new_handler Widget::set_new_handler(std::new_handler p) throw() {
  std::new_handler oldHandler = currentHandler;
  currentHandler = p;
  return oldHandler;
}
```

And to implement `operator new`, we have an RAII class for holding the current handler.
```cpp
class NewHandlerHolder {
public:
  explicit NewHandlerHolder(std::new_handler nh)    // acquire current
  :handler(nh) {}                                   // new-handler

  ~NewHandlerHolder()                               // release it
  { std::set_new_handler(handler); }
private:
  std::new_handler handler;                         // remember it

  NewHandlerHolder(const NewHandlerHolder&);        // prevent copying
  NewHandlerHolder&                                 // (see Item 14)
   operator=(const NewHandlerHolder&);
};
```

And `operator new`.
```cpp
void * Widget::operator new(std::size_t size) throw(std::bad_alloc)
{
  NewHandlerHolder                              // install Widget's
   h(std::set_new_handler(currentHandler));     // new-handler

  return ::operator new(size);                  // allocate memory
                                                // or throw

}                                               // restore global
                                                // new-handler
```

And client code
```cpp
void outOfMem();                   // decl. of func. to call if mem. alloc.
                                   // for Widget objects fails

Widget::set_new_handler(outOfMem); // set outOfMem as Widget's
                                   // new-handling function

Widget *pw1 = new Widget;          // if memory allocation
                                   // fails, call outOfMem

std::string *ps = new std::string; // if memory allocation fails,
                                   // call the global new-handling
                                   // function (if there is one)

Widget::set_new_handler(0);        // set the Widget-specific
                                   // new-handling function to
                                   // nothing (i.e., null)

Widget *pw2 = new Widget;          // if mem. alloc. fails, throw an
                                   // exception immediately. (There is
                                   // no new- handling function for
                                   // class Widget.)
```

Then if we want to reuse this piece, we make it a templated base
```cpp
template<typename T>              // "mixin-style" base class for
class NewHandlerSupport{          // class-specific set_new_handler
public:                           // support
  static std::new_handler set_new_handler(std::new_handler p) throw();
  static void * operator new(std::size_t size) throw(std::bad_alloc);

  ...                             // other versions of op. new —
                                  // see Item 52
private:
  static std::new_handler currentHandler;
};

template<typename T>
std::new_handler
NewHandlerSupport<T>::set_new_handler(std::new_handler p) throw() {
  std::new_handler oldHandler = currentHandler;
  currentHandler = p;
  return oldHandler;
}

template<typename T>
void* NewHandlerSupport<T>::operator new(std::size_t size)
  throw(std::bad_alloc) {
  NewHandlerHolder h(std::set_new_handler(currentHandler));
  return ::operator new(size);
}

// this initializes each currentHandler to null
template<typename T>
std::new_handler NewHandlerSupport<T>::currentHandler = 0;
```
With this base, to add class-specific `set_new_handler` support, we make `Widget` inherit from `NewHandlerSupport<Widget>`.
```cpp
class Widget: public NewHandlerSupport<Widget> {
  ...                          // as before, but without declarations for
};                             // set_new_handler or operator new
```
This may look weird of `Widget` inheriting from `Template<Widget>`, but note that `NewHandlerSupport<T>` does not use `T`, all we need is another copy of `NewHandlerSupport`, in particular, its static data member `currentHandler`, and the template only differentiates different classes.

This pattern of `Widget` inheriting from `Template<Widget>` is called **curiously recurring template pattern** (CRTP).

This pattern could easily lead to multiple inheritance, about which you'll want to consult Item 40.

In the standards before 1993 `new` returns `null` when it's unable to allocate the requested memory.
The standardization committee does not want to abandon the test-for-null codebase before throwing `bad_alloc` is standardized, so they provided alternative forms that does failure yields null.
These are the `nothrow` forms where they employ `nothrow` objects in `<new>`.

```cpp
class Widget { ... };
Widget *pw1 = new Widget;                 // throws bad_alloc if
                                          // allocation fails

if (pw1 == 0) ...                         // this test must fail

Widget *pw2 = new (std::nothrow) Widget;  // returns 0 if allocation for
                                          // the Widget fails

if (pw2 == 0) ...                         // this test may succeed
```
In `new (std::nothrow) Widget`, two things happen: the `nothrow` version of `operator new` is called to allocate enough memory for `Widget` object, and if that fails, `operator new` returns `null` pointer.
If it succeeds, `Widget` ctor is called, and it may decide to allocate more memory itself, and that allocation is not constrained to use `nothrow new`, and if that allocation (ctor) throws, this expression `new (std::nothrow) Widget` still throws.
In all likelihood, you won't need to use `new (std::nothrow) Widget`.

**Takeaways**
* `set_new_handler` allows you to specify a function to be called when memory allocation requests cannot be satisfied
* `nothrow new` is of limited utility, because it applies only to memory allocation; subsequent constructor calls may still throw exceptions

### Understand when it makes sense to replace `new` and `delete`

Why would anybody want to replace the default `new` and `delete`?
* **To detect usage errors**: if `new` and `delete` keeps track of allocated addresses, it'd be easier to tell double free. To detect overruns (writing beyond the end of a block) and underruns (writing before the start of a block), which you can do via `new` over-allocating a block to start with to accommodate certain byte patterns, and `delete` checks if those patterns are violated)
* **To improve efficiency**: the default needs to accommodate all patterns, large small frequent infrequent, and they need to worry about heap fragmentation. They work reasonably well for everybody, but really well for nobody. If you understand the memory usage pattern of your program, a custom `new` and `delete` can be much faster and use less memory
* **To collect usage statistics**: to understand the memory usage pattern of your program. How big are the sizes, how long are their lifetimes? FIFO, LIFO, or mostly random? How the pattern changes over time? High watermark?

Here is a `new` that has byte patterns to check for underrun / overrun.
```cpp
static const int signature = 0xDEADBEEF;

typedef unsigned char Byte;

// this code has several flaws—see below
void* operator new(std::size_t size) throw(std::bad_alloc) {
  using namespace std;

  size_t realSize = size + 2 * sizeof(int);    // increase size of request so2
                                               // signatures will also fit inside

  void *pMem = malloc(realSize);               // call malloc to get theactual
  if (!pMem) throw bad_alloc();                // memory

  // write signature into first and last parts of the memory
  *(static_cast<int*>(pMem)) = signature;
  *(reinterpret_cast<int*>(static_cast<Byte*>(pMem)+realSize-sizeof(int))) =
  signature;

  // return a pointer to the memory just past the first signature
  return static_cast<Byte*>(pMem) + sizeof(int);
}
```
One issue is that this `operator new` does not conform with the convention: e.g. it doesn't call `new_hanlde` in a loop in case of `bad_alloc`.
Item 51 discusses that in more details.
Here look at a more subtle issue, **alignment**: many architectures require data of particular types be placed at particular kinds of addresses. (E.g. an architecture might require that pointers occur at addresses that are a multiple of four (four-byte-aligned), or doubles appear at a multiple of eight (eight-byte-aligned)).
Failure to follow such conventions may lead to hardware exceptions at runtime, or slower execution.

C++ requires that all `operator new`s return pointers that are suitably aligned for any data type.
`malloc` labors under the same requirement, so returning a pointer allocated by `malloc` is safe, however in our sample we are returning the result of `malloc` offset by an `int`.
On a machine where `int` is 4 bytes, and `double` needs to be 8-bytes aligned, we'd probably return a pointer with improper alignment.
(`tr1` adds support for discovering type-specific alignment requirements)

Writing a custom memory manager that almost works is pretty easy. Writing one that works well is a lot harder. As a general rule, I suggest you not attempt it unless you have to.

In many cases, you don't, compilers / tools may have a flag to facilitate debugging and logging functionality in their memory management functions.

Or you can use open source memory managers, `Pool` from boost, e.g.
`Pool` deals with a large number of small objects well.

To summarize when you might want to customize `new` and `delete` in more details:
* To detect usage errors
* To collect statistics about the use of dynamically allocated memory
* To increase the speed of allocation and deallocation (profile before you do such!)
* To reduce the space overhead of default memory management
* To compensate for suboptimal alignment in the default allocator
* To cluster objects near one another: to reduce page faults (`placement new` in item 52)
* To obtain unconventional behaviors: e.g. you want to work with shared memory but only have a C API to do it, you could provide a C++ wrapper via custom `new` `delete` that call into the C API

**Takeaways**
* There are many valid reasons for writing custom versions of new and delete, including improving performance, debugging heap usage errors, and collecting heap usage information

### Adhere to conventions when writing `new` and `delete`

Implementing an `operator new` requires having the right return value, calling `new handle` when there isn't sufficient memory, and coping with requests for no memory.
You'll also want to avoid the normal form of `new`.

Pseudocode for a non-member new then looks like
```cpp
void * operator new(std::size_t size) throw(std::bad_alloc)
{                                      // your operator new might
  using namespace std;                 // take additional params

  if (size == 0) {                     // handle 0-byte requests
    size = 1;                          // by treating them as
  }                                    // 1-byte requests, simple and effective

  while (true) {
    attempt to allocate size bytes;

    if (the allocation was successful)
       return (a pointer to the memory);

    // allocation was unsuccessful; find out what the
    // current new-handling function is:
    // unfortunately, there is no way to get at the new-handling function
    // pointer directly, so you have to call set_new_handler to find out what it
    // is
    // this is no longer the case in C++11: C++11 has get_new_handler
    new_handler globalHandler = set_new_handler(0);
    set_new_handler(globalHandler);
    // in a multi-threaded context, you probably need some form of lock for
    // these two operations

    if (globalHandler) (*globalHandler)();
    else throw std::bad_alloc();
  }
}
```
Now given this infinite loop in conventional `operator new`, Item 49's requirement on `new handler` is clear:
`new handler` needs to make more memory available; install a different new-handler; deinstall the new-handler; throw an exception of or derived from bad_alloc; or fail to return

Member `operator new` is inherited by derived classes.
This may have interesting implications. Consider this code
```cpp
class Base {
public:
  static void * operator new(std::size_t size) throw(std::bad_alloc);
  ...
};

class Derived: public Base                // Derived doesn't declare
{ ... };                                  // operator new

Derived *p = new Derived;                 // calls Base::operator new!
```
It's possible that `Base`'s `operator new` is geared towards allocating `sizeof(Base)`, and to avoid allocating an unexpected amount of memory with `Base`'s `operator new`, we could do this
```cpp
void * Base::operator new(std::size_t size) throw(std::bad_alloc) {
  if (size != sizeof(Base))               // if size is "wrong,"
     return ::operator new(size);         // have standard operator
                                          // new handle the request

  ...                                     // otherwise handle
                                          // the request here
}
```

If you'd like to control memory allocation for arrays on a per-class basis, you need to implement array new: `operator new[]` as well.

You can't assume inside `operator new[]` the size of each object is `sizeof(Base)`, and you can't assume the number of objects is `requestedSize / sizeof(Base)`. And the size passed to `operator new[]` may ask for more memory than filled with objects, as dynamically allocated arrays may include extra space to store number of elements allocated (Item 16).

For `operator delete`, C++ guarantees it needs to be safe to delete `nullptr` (`NULL` as before 11), and you could honor this guarantee with pseudocode like this
```cpp
void operator delete(void *rawMemory) throw() {
  if (rawMemory == 0) return;            // do nothing if the null
                                         // pointer is being deleted

  deallocate the memory pointed to by rawMemory;
}
```
The member version of this is simple, too, except that you need to check the size of the deleted object, in case a `Base::operator delete` tries to delete an object of `Derived` 
```cpp
class Base {                            // same as before, but now
public:                                 // operator delete is declared

  static void * operator new(std::size_t size) throw(std::bad_alloc);
  static void operator delete(void *rawMemory, std::size_t size) throw();
  ...
};
void Base::operator delete(void *rawMemory, std::size_t size) throw()
{
  if (rawMemory == 0) return;           // check for null pointer

  if (size != sizeof(Base)) {           // if size is "wrong,"
     ::operator delete(rawMemory);      // have standard operator
     return;                            // delete handle the request
  }
  deallocate the memory pointed to by rawMemory;
  return;
}
```
Interestingly, the `size_t` C++ pass to `delete` may not be correct, if the object being deleted was derived from a class lacking a virtual dtor (Item 7).

**Takeaways**
* `operator new` should contain an infinite loop trying to allocate memory, should call the `new handler` if it can't satisfy a memory request, and should handle requests for zero bytes. Class-specific versions should handle requests for larger blocks than expected
* `operator delete` should do nothing if passed a pointer that is null. Class-specific versions should handle blocks that are larger than expected

### Write `placement delete` if you write `placement new`

Recall that when you call this
```cpp
Widget *pw = new Widget;
```
Two functions are called: one to `operator new` to allocate memory, a second to `Widget`'s default constructor.

If step 1 succeeds but 2 fails, C++ runtime system needs to deallocate the memory from step 1, as `pw` is never assigned.

The runtime system needs to figure out which `operator delete` to call, since there may be many: suppose you have global `new` and `delete`, and also class-specific, non-normal forms of `new` and `delete`. Like
```cpp
class Widget {
public:
  ...
  static void* operator new(std::size_t size,              // non-normal
                            std::ostream& logStream)       // form of new
    throw(std::bad_alloc);

  static void operator delete(void *pMemory                // normal class-
                              std::size_t size) throw();   // specific form
                                                           // of delete
  ...
};
```
When an `operator new` takes extra parameters (other than the mandatory `size_t`), the operator is known as `placement new`.

A particularly useful version of `placement new` is one that takes a pointer specifying where the object should be placed.
Like this.
```cpp
void* operator new(std::size_t, void *pMemory) throw();   // "placement
                                                          // new"
```
This version is in std's `<new>`, and it's used inside `vector` to create objects in `vector`'s unused capacity.
The term `placement new` is overridden, when people talk about it, they are usually referring to this particular function.

Now if we go back to check how `Widget`'s `placement new` can be problematic, say we have
```cpp
Widget *pw = new (std::cerr) Widget; // call operator new, passing cerr as
                                     // the ostream; this leaks memory
                                     // if the Widget constructor throws
```
When deleting `pw` due to exception in step 2 (as described above, step 1 is `operator new` and step 2 is ctor), the runtime system looks for an `operator delete` taking the same number and type of extra arguments as `operator new`.

In this case, the runtime system would settle on this, a `placement delete`
```cpp
void operator delete(void *, std::ostream&) throw();
```

But `Widget`'s placement `delete` has a different interface, meaning in this case, no `operator delete` is called if `Widget` throws an exception!

To eliminate this possible leak, `Widget` needs to be like
```cpp
class Widget {
public:
  ...

  static void* operator new(std::size_t size, std::ostream& logStream)
    throw(std::bad_alloc);
  static void operator delete(void *pMemory) throw();

  static void operator delete(void *pMemory, std::ostream& logStream)
    throw();
  ...
};
```
Now if step 2 throws, the 2nd `delete` handles it.
But if no exceptions in step 2 and we get a `delete pw` in client code, this call never calls the placement version, meaning to forestall memory leaks associated with `placement new`, you need to provide both the `placement delete` and the normal version (without providing the normal version, the `placement delete` would hide the global regular version).

And because of hiding, if you want the client to be able to use the normal `operator new` as well as the placement version, you'll need to have both.
```cpp
class Base {
public:
  ...

  static void* operator new(std::size_t size,           // this new hides
                            std::ostream& logStream)    // the normal
    throw(std::bad_alloc);                              // global forms
  ...
};

Base *pb = new Base;                        // error! the normal form of
                                            // operator new is hidden

Base *pb = new (std::cerr) Base;            // fine, calls Base's
                                            // placement new
```
Similarly, `operator new`s in derived classes hide both global and inherited versions of `operator new`.
```cpp
class Derived: public Base {                   // inherits from Base above
public:
  ...

  static void* operator new(std::size_t size)  // redeclares the normal
      throw(std::bad_alloc);                   // form of new
  ...
};
Derived *pd = new (std::clog) Derived;         // error! Base's placement
                                               // new is hidden

Derived *pd = new Derived;                     // fine, calls Derived's
                                               // operator new
```

By default, C++ offers the following forms of `new` at global scope:
```cpp
void* operator new(std::size_t) throw(std::bad_alloc);      // normal new

void* operator new(std::size_t, void*) throw();             // placement new

void* operator new(std::size_t,                             // nothrow new —
                   const std::nothrow_t&) throw();          // see Item 49
```

If you declare any `operator new`s in your class, you'll hide all three.
Unless you mean to forbid the clients from using these, make sure to make these available in addition to versions you declare.

For each `operator new` you make available, of course, be sure to offer the corresponding `operator delete`, too.
If you want these functions to behave in the usual way, have your class-specific versions call the global versions.

Like this base class
```cpp
class StandardNewDeleteForms {
public:
  // normal new/delete
  static void* operator new(std::size_t size) throw(std::bad_alloc)
  { return ::operator new(size); }
  static void operator delete(void *pMemory) throw()
  { ::operator delete(pMemory); }

  // placement new/delete
  static void* operator new(std::size_t size, void *ptr) throw()
  { return ::operator new(size, ptr); }
  static void operator delete(void *pMemory, void *ptr) throw()
  { return ::operator delete(pMemory, ptr); }

  // nothrow new/delete
  static void* operator new(std::size_t size, const std::nothrow_t& nt) throw()
  { return ::operator new(size, nt); }
  static void operator delete(void *pMemory, const std::nothrow_t&) throw()
  { ::operator delete(pMemory); }
};
```
And a derived class of this can avoid hiding via `using` statements.
```cpp
class Widget: public StandardNewDeleteForms {           // inherit std forms
public:
   using StandardNewDeleteForms::operator new;          // make those
   using StandardNewDeleteForms::operator delete;       // forms visible

   static void* operator new(std::size_t size,          // add a custom
                             std::ostream& logStream)   // placement new
     throw(std::bad_alloc);

   static void operator delete(void *pMemory,           // add the corres-
                               std::ostream& logStream) // ponding place-
     throw();                                           // ment delete
  ...
};
```

**Takeaways**
* When you write a placement version of `operator new`, be sure to write the corresponding placement version of `operator delete`. If you don't, your program may experience subtle, intermittent memory leaks
* When you declare placement versions of `new` and `delete`, be sure not to unintentionally hide the normal versions of those functions
