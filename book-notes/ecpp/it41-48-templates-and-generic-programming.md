# Templates and generic programming

### Understand implicit interfaces and compile-time polymorphism

We are familiar with explicit interfaces (one that is explicitly visible in the source code), and runtime polymorphism (via virtual function).

In the world of templates and generic programming, implicit interface and compile time polymorphism move to the fore.

Consider this code
```cpp
template<typename T>
void doProcessing(T& w) {
  if (w.size() > 10 && w != someNastyWidget) {
     T temp(w);
     temp.normalize();
     temp.swap(w);
  }
}
```
`w`'s type `T` must support `size`, `normalize` and `swap`, copyctor, and comparison for inequality.
While these aren't yet complete, but they suffice for now to demonstrate the **implicit interface** `T` must support.

To make calls involving `w` succeed we need to instantiate the template at compile time.
Because instantiating function templates with different template parameters leads to different functions being called, this is known as **compile time polymorphism**.

An implicit interface is not based on function signatures, rather, it consists of valid expressions: e.g. `T` must often a `size` member function that returns an integral.
Actually not necessarily an integral, rather, as long as `size()` returns a type `X` that has `operator<(Y, int)` defined, where `X` can implicit convert to `Y`, then we can instantiate this call.

Just as you can't use an object in a way contradictory to the explicit interface its class offers (the code won't compile), you can't try to use an object in a template unless that object supports the implicit interface the template requires (again, the code won't compile).

**Takeaways**
* Both classes and templates support interfaces and polymorphism
* For classes, interfaces are explicit and centered on function signatures. Polymorphism occurs at runtime through virtual functions
* For template parameters, interfaces are implicit and based on valid expressions. Polymorphism occurs during compilation through template instantiation and function overloading resolution

### Understand the two meanings of typename

