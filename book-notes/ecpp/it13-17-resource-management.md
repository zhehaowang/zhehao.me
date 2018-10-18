# Resource management

A resource is something that, once you're done using it, you need to return to the system. If you don't, bad things happen.

In C++, commonly used resources include dynamically allocated memory, file descriptors, mutex locks, database connections, network sockets, etc.
It's important that it be released when you are done using them.

Trying to ensure this by hand is difficult under any conditions, but when you consider exceptions, functions with multiple return paths, and maintenance programmers modifying software without fully comprehending the impact of their changes, it becomes clear that ad hoc ways of dealing with resource management aren't sufficient.

### Use objects to manage resources

Suppose we have this factory function
```cpp
Investment* createInvestment();  // return ptr to dynamically allocated
                                 // object in the Investment hierarchy;
                                 // the caller must delete it
                                 // (parameters omitted for simplicity)
```
For creating objects of different child classes of the Investment interface.

And we have this code that uses the above
```cpp
void f() {
  Investment *pInv = createInvestment();         // call factory function

  ...                                            // use pInv

  delete pInv;                                   // release object
}
```
There are several ways pInv won't be deleted: a premature return statement, or an exception before the delete.
Careful programming may work around those issues, but think about how code changes over time, relying on f getting to the delete statement isn't always viable.

So instead, we put resource management inside an object, and we can rely on C++'s automatic dtor invocation to make sure that the resources are released.

In this case, an auto\_ptr is well suited for the job.
```cpp
void f() {
  std::auto_ptr<Investment> pInv(createInvestment());  // call factory
                                                       // function

  ...                                                  // use pInv as
                                                       // before

}                                                      // automatically
                                                       // delete pInv via
                                                       // auto_ptr's dtor
```

This example demonstrates two critical aspects of using objects to manage resources:
* Resources are acquired and immediately turned over to resource managing objects. This is where the name Resource Acquisition Is Initialization (RAII) comes from, because it's so common to acquire a resource and initialize a resource-managing object in the same statement.
* Resource managing objects use their dtors to ensure resources are released. Because destructors are called automatically when an object is destroyed (e.g., when an object goes out of scope), resources are correctly released, regardless of how control leaves a block. Things can get tricky when releasing can result in an exception, which is addressed in Item 8.

Because auto\_ptr deletes what it points to when it's deleted, it's important to never have two auto\_ptrs pointing to the same object.
To prevent such problems, auto\_ptr has a copycon / copy assignment that does not take in const rhs, and sets rhs to null when 'copying' from them, so that the ownership is 'moved' to the ctor'ed or assigned auto\_ptr. An example.
```cpp
std::auto_ptr<Investment>                 // pInv1 points to the
  pInv1(createInvestment());              // object returned from
                                          // createInvestment

std::auto_ptr<Investment> pInv2(pInv1);   // pInv2 now points to the
                                          // object; pInv1 is now null

pInv1 = pInv2;                            // now pInv1 points to the
                                          // object, and pInv2 is null
```

STL containers require that their content exhibits normal copying behavior, thus containers of auto\_ptr are not allowed.

An alternative is a reference counting smart pointer, std::shared\_ptr in C++11, whose copying works 'as expected', and can be used in STL containers.

Both auto\_ptr and shared\_ptr use delete and not delete[] in their dtor, which means using them to manage an array is a bad idea, but one that will compile.
```cpp
std::auto_ptr<std::string>                       // bad idea! the wrong
  aps(new std::string[10]);                      // delete form will be used

std::shared_ptr<int> spi(new int[1024]);    // same problem
```

Dynamically allocated arrays in C++ don't have corresponding auto\_ptr and shared\_ptr, because vector and string can almost always replace dynamically allocated arrays.
If you want them, look to boost::scoped\_array and boost::shared\_array.

This item indicates if you are releasing resources manually (using delete other than in resource managing classes), you are doing something wrong.

For managing other resources, you often have to craft your own resource managing classes (think a lockguard), in which case more subtleties to be considered are described in Item 14 and 15.

As a final comment, createInvestment's raw pointer return type is an invitation to a resource leak, because it's so easy for callers to forget to call delete on the pointer they get back.
The interface change to createInvestment is discussed in Item 18.

**Takeaways**
* To prevent resource leaks, use RAII objects that acquire resources in their constructors and release them in their destructors.
* Two commonly useful RAII classes are std::shared\_ptr and std::unique\_ptr. std::shared\_ptr is usually the better choice, because its behavior when copied is intuitive. Copying an std::auto_ptr sets it to null. For the complete up-to-date argument, refer to Items 18 and 19 in emcpp.

### Think carefully about copying behavior in resource-managing classes

There are cases where you need to create your own resource managing classes, say, you are using C API mutex, and creating an RAII class to wrap around it. For example,
```cpp
class Lock {
public:
  explicit Lock(Mutex *pm)
  : mutexPtr(pm)
  { lock(mutexPtr); }                          // acquire resource

  ~Lock() { unlock(mutexPtr); }                // release resource

private:
  Mutex *mutexPtr;
};

// to be used like this
Mutex m;                    // define the mutex you need to use

...

{                           // create block to define critical section
Lock ml(&m);               // lock the mutex

...                         // perform critical section operations

}                           // automatically unlock mutex at end
                            // of block
```

But what should happen if a lock object is copied?
```cpp
Lock ml1(&m);                      // lock m

Lock ml2(ml1);                     // copy ml1 to ml2—what should
                                   // happen here?
```
"What should happen when copied" is a question every RAII class author should confront. 
The usual answers are

* Prohibit copying, e.g. it rarely makes sense to copy a synchronization primitive like mutex

Refer to emcpp item 11. This is the behavior of std::unique\_ptr

* Reference count the underlying resource, e.g. a std::shared\_ptr

Often an RAII class can implement the reference counting behavior with a std::shared\_ptr data member.
If we want to allow the mutex to be reference counted, we could make the data member a std::shared\_ptr<Mutex> but with a custom deleter (since when the count drops to 0, we don't want to destroy the mutex but rather unlock it).
With TR1 it looks something like this:
```cpp
class Lock {
public:
  explicit Lock(Mutex *pm)       // init shared_ptr with the Mutex
  : mutexPtr(pm, unlock)         // to point to and the unlock func
  {                              // as the deleter

    lock(mutexPtr.get());   // see Item 15 for info on "get"
  }
private:
  std::tr1::shared_ptr<Mutex> mutexPtr;    // use shared_ptr
};                                         // instead of raw pointer
```
In this case, note the absence of a custom dtor.

* Copy the underlying resource

Sometimes you can have as many copies of the managed resource as you like, in which case the copy operations perform a deep copy of the managed resource.
Some implementations of std::string does this: string class contains pointer to heap memory, and both the pointer and the heap memory are copied when a string copy is made.

* Transfer ownership of the managed resource

Occasionally you may want to transfer ownership to the copied object when 'copying'.
This is the behavior of std::auto\_ptr.

**Takeaways**
* Copying an RAII object entails copying the resource it manages, so the copying behavior of the resource determines the copying behavior of the RAII object
* Common RAII class copying behaviors are disallowing copying and performing reference counting, but other behaviors (deep copy and transfer ownership) are possible

### Provide access to raw resources in resource managing classes

