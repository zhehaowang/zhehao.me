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


