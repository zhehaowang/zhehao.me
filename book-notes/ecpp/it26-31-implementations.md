# Implementations

Coming up with appropriate definitions for your classes and declaration for your functions is often times lion's share of the battle, but there are still things in implementation to watch out for.

### Postpone variable definitions as long as possible

Unused variables come with the cost of ctors and dtors. Like this function
```cpp
// this function defines the variable "encrypted" too soon
std::string encryptPassword(const std::string& password)
{
  using namespace std;

  string encrypted;

  if (password.length() < MinimumPasswordLength) {
      throw logic_error("Password is too short");
  }
  ...                        // do whatever is necessary to place an
                             // encrypted version of password in encrypted
  return encrypted;
}
```
The object encrypted isn't completely unused, only if an exception is thrown. Why not move it after the length check instead?

With that the code is still not as tight as it might be: encrypted is defined without initialization.
Say you are going to want encrypted to hold the value of password initially, then given to an encrypt call
```cpp
void encrypt(std::string& s);             // encrypts s in place

// instead of
std::string encrypted;                // default-construct encrypted
encrypted = password;                 // assign to encrypted
encrypt(encrypted);

// just do
std::string encrypted(password);
encrypt(encrypted);
```

Not only should you postpone a variable's definition until right before you have to use the variable, you should also try to postpone the definition until you have initialization arguments for it.

What about loops?
```cpp
// Approach A: define outside loop   // Approach B: define inside loop

Widget w;
for (int i = 0; i < n; ++i){         for (int i = 0; i < n; ++i) {
  w = some value dependent on i;       Widget w(some value dependent on i);
  ...                                  ...
}                                    }

// 1 ctor + 1 dtor + n assignments   // n ctor + n dtor
```

A is generally more efficient if you know an assignment costs less than a ctor-dtor pair.
A also makes w visible in larger scope, something that's contrary to program comprehensibility and maintainability.

As a result, you should default to B unless you know assignment is less expensive than ctor-dtor pair and you are dealing with a performance sensitive part of your code.

**Takeaways**
* Postpone variable definitions as long as possible. It increases program clarity and improves program efficiency

### Minimize casting

In theory, if your program compiles cleanly, it's not trying to perform any unsafe or nonsensical operations on any objects.

This is a valuable guarantee, you don't want to forego it lightly.

Unfortunately casts can subvert the type system, potentially leading to some subtle issues.

In C, Java, C\# casting is generally more necessary and less dangerous in C++.

Casts can look like these
```cpp
// C style
(T) expression                      // cast expression to be of type T
T(expression)                       // cast expression to be of type T
// no difference between the two, purely where you put the parentheses

// C++ style
const_cast<T>(expression)
dynamic_cast<T>(expression)
reinterpret_cast<T>(expression)
static_cast<T>(expression)
```

Each serves a distinct purpose:
* `const_cast` casts away the constness, and is the only cast that can achieve this.
* `dynamic_cast` is primarily used to perform safe downcasting, i.e. to determine whether an object is of a particular type in the inheritance hierarchy. It is the only cast that cannot be performed with old style casts, and the only cast that may incur a significant runtime cost.
* `reinterpret_cast` is intended for low level casts that yield implementation-dependent (i.e. unportable) results. E.g. casting pointer to int. Such casts should be rare outside low-level code.
* `static_cast` can be used to force implicit conversion, (e.g. non-const object to const, `int` to `double`, etc), also the reverse of many such conversions (e.g. `void*` to typed pointers, pointer-to-base to pointer-to-derived), though it cannot cast const to non-const.

C++ style casts are preferred over C style ones.
They are easier to identify in code, and the more narrowly specified purpose makes it possible for compilers to detect errors.

About the only time this book uses old style cast is when calling an explicit ctor to pass an object to a function. Like this.
```cpp
class Widget {
public:
  explicit Widget(int size);
  ...
};

void doSomeWork(const Widget& w);

doSomeWork(Widget(15));                    // create Widget from int
                                           // with C-style cast

doSomeWork(static_cast<Widget>(15));       // create Widget from int
                                           // with C++-style cast
```

The belief that casts do nothing but tell compilers to treat one type as another is wrong.
Type conversion of any kind (explicit via casts or implicit by compilers) often leads to code that is executed at runtime.
E.g. this
```cpp
int x, y;
...
double d = static_cast<double>(x)/y;           // divide x by y, but use
                                               // floating point division
```
Converting from int to double will most likely generate code due to different underlying representations.

Now what about this
```cpp
class Base { ... };

class Derived: public Base { ... };

Derived d;

Base *pb = &d;                         // implicitly convert Derived* ⇒ Base*
```
Here we're just creating a base class pointer to a derived class object, but sometimes, the two pointer values will not be the same.
When that's the case, an offset is applied at runtime to the Derived* pointer to get the correct Base* pointer value.

When do such cases happen? It can't happen in C, C\# or Java, but it does happen in C++, with multiple inheritance.
It can happen in single inheritance, too, so avoid making assumptions on how things are laid out, and performing casts based on such assumptions.
E.g., casting object addresses to char* pointers and then using pointer arithmetic on them almost always yields undefined behavior.

Object layout is compiler specific, so by making the above assumptions, you are making your code unportable.

An interesting thing about casts is that they may seem to do the right thing, but in fact they don't.
Consider this example of calling base method first in a derived class's method impl.
```cpp
class Window {                                // base class
public:
  virtual void onResize() { ... }             // base onResize impl
  ...
};

class SpecialWindow: public Window {          // derived class
public:
  virtual void onResize() {                   // derived onResize impl;
    static_cast<Window>(*this).onResize();    // cast *this to Window,
                                              // then call its onResize;
                                              // this doesn't work!

    ...                                       // do SpecialWindow-
  }                                           // specific stuff

  ...

};
```
This code does call the base class's `onResize` method, but not on `this` object!
Instead, the cast creates a new, temporary copy of the base class part of `*this`, then invokes onResize on the copy!
This can easily lead to object being in an invalid state.

The solution is not to use cast but express what you really want to say:
```cpp
class SpecialWindow: public Window {
public:
  virtual void onResize() {
    Window::onResize();                    // call Window::onResize
    ...                                    // on *this
  }
  ...

};
```

This suggests wanting to cast, especially `dynamic_cast`, could be a sign that you are doing things the wrong way.

Before diving into details about `dynamic_cast`, know that many implementations of `dynamic_cast` can be slow.
E.g. an implementation based on string comparison of class names (think an object in an inheritance hierarchy four levels deep, each `dynamic_cast` in this chain would be doing 4 `strcmp`s)

The need for `dynamic_cast` generally arises because you want to perform derived class operations on what you believe to be a derived class object, but you only have a pointer or reference to base to work with.

To avoid this,
* one way is to have containers contain the derived object pointers instead of base object pointers, though this would mean having separate containers for different objects in the inheritance chain.
* an alternative is to declare this virtual function in base as well, just that it's noop in base.

Neither of these approaches (type safe containers, or moving virtual functions up the hierarchy) are universally applicable, but they offer alternatives to `dynamic_cast`.

One thing you definitely want to avoid is cascading `dynamic_cast`, like in this inheritance hierarchy of Window, SpecialWindow1, SpecialWindow2, SpecialWindow3,
```cpp
typedef std::vector<std::shared_ptr<Window> > VPW;
VPW winPtrs;

for (VPW::iterator iter = winPtrs.begin(); iter != winPtrs.end(); ++iter)
{
  if (SpecialWindow1 *psw1 =
       dynamic_cast<SpecialWindow1*>(iter->get())) { ... }

  else if (SpecialWindow2 *psw2 =
            dynamic_cast<SpecialWindow2*>(iter->get())) { ... }

  else if (SpecialWindow3 *psw3 =
            dynamic_cast<SpecialWindow3*>(iter->get())) { ... }

  ...
}
```
Such code generates code that is big and slow and brittle (in the sense that each time the inheritance hierarchy changes, the code needs to be reinspected).

Good C++ uses few casts, but it's generally not practical to completely get rid of them. The static cast from `int` to `double` could be considered fine, but still avoidable (e.g. creating a `double` variable from the `int`)

Like most suspicious constructs, casts should be isolated as much as possible, typically hidden inside functions whose interfaces shield callers from the grubby work being done inside.

**Takeaways**
* Avoid casts whenever practical, especially dynamic_casts in performance-sensitive code. If a design requires casting, try to develop a cast-free alternative
* When casting is necessary, try to hide it inside a function. Clients can then call the function instead of putting casts in their own code
* Prefer C++-style casts to old-style casts. They are easier to see, and they are more specific about what they do

### Avoid returning handles to object internals

