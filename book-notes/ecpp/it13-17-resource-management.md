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

In the perfect world you only need to interact with resource-managing objects, but in reality many APIs refer to the managed resource directly.
Say, a raw pointer is wanted but we have a shared\_ptr / unique\_ptr. Both of those would offer explicit conversion (via .get()) and implicit conversion (via operator\* and operator->) to access the raw pointer.

User custom resource management classes often demonstrate similar approaches: explicit and implicit.
For example in this Font resource management class that wraps around an underlying FontHandle.
```cpp
// given C API that works with FontHandle

FontHandle getFont();               // from C API—params omitted
                                    // for simplicity

void releaseFont(FontHandle fh);    // from the same C API

// we have the following RAII class, and to accommodate other APIs
// working directly with FontHandle, we have the explicit and the
// implicit conversions approach

class Font {                           // RAII class
public:
  explicit Font(FontHandle fh)         // acquire resource;
  : f(fh)                              // use pass-by-value, because the
  {}                                   // C API does

  ~Font() { releaseFont(f); }          // release resource

  // explicit
  FontHandle get() const { return f; } // explicit conversion function
  // downside is explicit calls to get() function is required everywhere
  // FontHandle is expected

  // alternative, implicit approach, note the operator overload!
  operator FontHandle() const { return f; } // implicit conversion function
  // downside is increased chance of error
private:
  FontHandle f;                        // the raw font resource
};

...
Font f1(getFont());

// downside of the explicit approach:
useFontHandle(f1.get());  // get has to be called explicitly

// downside of the implicit approach:
FontHandle f2 = f1;                 // oops! meant to copy a Font
                                    // object, but instead implicitly
                                    // converted f1 into its underlying
                                    // FontHandle, then copied that
// Now the program has a FontHandle being managed by the Font object f1,
// but the FontHandle is also available for direct use as f2.
// That's almost never good. For example, when f1 is destroyed, the font
// will be released, and f2 will dangle.
```
The class's designer chooses whether an implicit or explicit approach is provided.
The guideline is in item 18: make interfaces easy to use correctly and hard to use incorrectly.

It may occur to you returning the resource being managed in resource management classes is contrary to encapsulation.
This is true, but not as disastrous: RAII classes don't exist to encapsulate something, but rather to ensure a particular action, resource release, takes place.

Well-designed classes hides what clients don't need to see, but makes available those things that clients honestly need to access.

**Takeaways**
* APIs often require access to raw resources, so each RAII class should offer a way to get at the resource it manages.
* Access may be via explicit conversion or implicit conversion. In general, explicit conversion is safer, but implicit conversion is more convenient for clients.

### Use the same form in corresponding uses of new and delete

Consider this code
```cpp
std::string *stringArray = new std::string[100];
...
delete stringArray;
```
This is undefined behavior, at least, 99 of the 100 strings are unlikely to be properly destroyed, because their dtor will probably never be called.

When you employ a new operation, two things happen:
First, memory is allocated (via a function named operator new—see Items 49 and 51).
Second, one or more constructors are called for that memory.

When you employ a delete expression, two things happen:
First, one or more destructors are called for the memory.
Second, the memory is deallocated (via a function named operator delete—see Item 51).

The big question for delete is how many objects reside in the memory being deleted?
Whose answer decides how many dtors should be called.

Actually the question is simpler: does the pointer being deleted point to a single object or an array of objects?
This is important because layouts are usually different, think of it like this (compilers aren't required to implement it like this, but some do):
```
Single Object:    | Object |
Array of Objects: | n | Object | Object | ... |
```

When you call delete, the only way for it to know whether the layout looks like the first or the second is for you to tell it: whether you use delete[], or delete.
Like this:
```cpp
std::string *stringPtr1 = new std::string;

std::string *stringPtr2 = new std::string[100];
...

delete stringPtr1;                       // delete an object

delete [] stringPtr2;                    // delete an array of objects
```
Using delete[] on stringPtr1, or using delete on stringPtr2 would cause undefined behaviors (e.g., too many or too few dtors called as the memory layout is not interpretted correctly.)

This is particularly important to bear in mind when you write a class that deals with dynamic allocation and provides multiple versions of ctor: all of them must use the same new or new X[], as otherwise, dtor cannot be implemented.

Also for those using typedefs, they should document which form of delete should be used.
```cpp
typedef std::string AddressLines[4];   // a person's address has 4 lines,
                                       // each of which is a string

// because AddressLines is an array underlying, resource allocated with
std::string *pal = new AddressLines;   // note that "new AddressLines"
                                       // returns a string*, just like
                                       // "new string[4]" would
// should use
delete [] pal;                         // fine
```
To avoid such confusions, abstain from typedefs for array types, e.g. AddressLines could be a vector\<string\>

**Takeaways**
* If you use [] in a new expression, you must use [] in the corresponding delete expression. If you don't use [] in a new expression, you mustn't use [] in the corresponding delete expression.

### Store new'ed objects in smart pointers in standalone statements

_This item illustrates the same issue as that of emcpp item 21._

Consider this code
```cpp
processWidget(std::shared_ptr<Widget>(new Widget), priority());
// vs
std::shared_ptr<Widget> pw(new Widget);  // store newed object
                                         // in a smart pointer in a
                                         // standalone statement

processWidget(pw, priority());           // this call won't leak

// the first version could leak due to compiler being given freedom to reorder
// 1. new Widget
// 2. std::shared\_ptr ctor
// 3. priority()
// as long as 2 comes after 1.
//
// now if compiler orders it as 1 3 2, and 3 throws, 2 won't get to manage the
// allocated resource from 1, and 1 causes a leak.
// so always prefer the second version.
```

**Takeaways**
* Store newed objects in smart pointers in standalone statements. Failure to do this can lead to subtle resource leaks when exceptions are thrown.
