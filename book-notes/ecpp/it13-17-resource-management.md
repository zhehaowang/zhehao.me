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



