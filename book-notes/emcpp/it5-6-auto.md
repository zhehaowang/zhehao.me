# Chap 2 auto

### Prefer auto to explicit type declarations

Upsides:
* Avoids variable uninitalized issue: auto variables have their type deduced from the initializer, so they must be initialized
* Avoids verbose variable declarations
* Can hold a closure in lieu of `std::function`
* Avoids potential implicit conversion you don't want: like `::size_type` to `int`, and the pairs in `unordered_map<K, V>` who are actually `pair<const K, V>` instead of `pair<K, V>` (unnecessary copycon calls!)
* Potentially can make refactoring easier

`std::function` is a template in C++11 that generalizes the idea of a function pointer (who can only point to functions), a `std::function` can refer to any callable object.
You specify the type (signature) of function you refer to when creating a std::function object.
Since lambda expressions yield callable objects, closures can be stored in `std::function` objects.

`std::function` object (comes at a fixed size, and if not big enough, heap allocate additional memory) typically uses more memory than the auto-declared object (takes as much space as the closure requires).
invoking a closure via a std::function object is almost certain to be slower than calling it via an auto-declared object. (implementation details that restrict inlining and yield indirect function calls)

Downsides:
* auto may deduce unexpected types. E.g. [item-2](it1-4-deducing-types.md#understand-auto-type-deduction)
```cpp
int x = 5;
auto y = (x);
```
* code readability. The counter argument would be that type inference is sufficiently understood, and IDE can always help.

**Takeaways**
* auto variables must be initialized, are generally immune to type mismatches that can lead to portability or efficiency problems, can ease the process of refactoring, and typically require less typing than variables with explicitly specified types.
* auto-typed variables are subject to the pitfalls described in Items 2 and 6.

### Use the explicitly typed initializer idiom when auto deduces undesired types

As a general rule, "invisible" proxy classes don't play well with auto.
Objects of such classes are often not designed to live longer than a single statement, so creating variables of those types tends to violate fundamental library design assumptions.

For example, consider this code
```cpp
Widget w;
bool highPriority = features(w)[5];  // is w high priority?
                                     // (features call returns a vector<bool>)
processWidget(w, highPriority);      // process w in accord
                                     // with its priority
```
Works just fine. Yet if we do
```cpp
Widget w;
auto highPriority = features(w)[5];
processWidget(w, highPriority);      // undefined behavior
```
Reason is in the type of highPriority, it's now std::vector<bool>::reference.
std::vector<T>[] operator returns a T&, except in the case of vector<bool> where it returns a vector<bool>::reference that can be implicitly casted to bool.
Reason for having this ::reference is that the underlying uses a compact bit-storage for vector<bool>. You can't return a reference to a bit in C++, so a proxy class ::reference is used instead 
So what happened instead is highPriority will be a reference to a temporary, and when we use it in processWidget the temporary is already destroyed and it causes undefined behavior.

Solution is to use explicitly typed initializer. Like
```cpp
auto highPriority = static_cast<bool>features(w)[5];
```

So when do we use explicitly typed initializer with auto?
* when auto will look at invisible proxy types.
(another common example is a Matrix class where the sum of 3 matrices is of type Sum<Sum<Matrix, Matrix>, Matrix> for performance reasons, and Sum<T, U> is a proxy class within Matrix) 
* when you intend to do a conversion. (e.g. double -> float)

**Takeaways**
* "Invisible" proxy types can cause auto to deduce the "wrong" type for an initializing expression.
* The explicitly typed initializer idiom forces auto to deduce the type you want it to have.
