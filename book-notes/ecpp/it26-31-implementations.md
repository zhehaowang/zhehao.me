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

Suppose you are working on a rectangle class represented by its upper left and lower right corners.
To keep a Rectangle object small, you decided to keep the extents of a rectangle in a class pointed to by a member in the Rectangle object.
```cpp
class Point {                      // class for representing points
public:
  Point(int x, int y);
  ...

  void setX(int newVal);
  void setY(int newVal);
  ...
};

struct RectData {                    // Point data for a Rectangle
  Point ulhc;                        // ulhc = " upper left-hand corner"
  Point lrhc;                        // lrhc = " lower right-hand corner"
};

class Rectangle {
  ...

private:
  std::shared_ptr<RectData> pData;   // see Item 13 for info on
};                                   // tr1::shared_ptr
```
Now you want to add functions in Rectangle to expose its points:
```cpp
class Rectangle {
public:
  ...
  Point& upperLeft() const { return pData->ulhc; }
  Point& lowerRight() const { return pData->lrhc; }
  ...
};
```
This design is self-contradictory: const member functions (meant for read-only use) exposed internal private data that can be modified by client.
In this case, ulhc and lrhc are effectively public.
Since ulhc and lrhc are stored outside the Rectangle class, const member functions of Rectangle can return references to them. (limitation of bitwise const)

Returning pointers, iterators demonstrates the same problem (breaking encapsulation): they are handles whose modification will affect internal members.

Similarly for private member functions, you should never have a public member function return a pointer to a private member function since if you do the access level of that private member function is practically public.

What about this?
```cpp
class Rectangle {
public:
  ...
  const Point& upperLeft() const { return pData->ulhc; }
  const Point& lowerRight() const { return pData->lrhc; }
  ...
};
```
Now clients cannot modify the returned and read-only is conveyed, but it can lead to dangling references.
What if the referred object disappears? Like this
```cpp
class GUIObject { ... };

const Rectangle                             // returns a rectangle by
  boundingBox(const GUIObject& obj);        // value; see Item 3 for why
                                            // return type is const

GUIObject *pgo;                             // make pgo point to
...                                         // some GUIObject

const Point *pUpperLeft =                   // get a ptr to the upper
  &(boundingBox(*pgo).upperLeft());         // left point of its
                                            // bounding box
```
`boundingBox(*pgo)` returns a temporary object that will be destroyed at the end of the statement.
In turn, pUpperLeft will be dangled at the end of the statement that created it.

This is why returning a reference / iterator / pointer to an internal part of the object is dangerous: what if the reference outlives the object?

This doesn't mean you should never return a handle, sometimes you have to, e.g. `operator[]` of `string` and `vector`.
But such functions are exceptions, not the rule.

**Takeaways**
* Avoid returning handles (references, pointers, or iterators) to object internals. This increases encapsulation, helps const member functions act const, and minimizes the creation of dangling handles.

### Strive for exception-safe code

Consider this code where we want to have this menu change background images in a threaded environment. 
```cpp
class PrettyMenu {
public:
  ...
  void changeBackground(std::istream& imgSrc);           // change background
  ...                                                    // image

private:

  Mutex mutex;                    // mutex for this object

  Image *bgImage;                 // current background image
  int imageChanges;               // # of times image has been changed
};
```
We could implement `changeBackground` like this
```cpp
void PrettyMenu::changeBackground(std::istream& imgSrc)
{
  lock(&mutex);                      // acquire mutex (as in Item 14)

  delete bgImage;                    // get rid of old background
  ++imageChanges;                    // update image change count
  bgImage = new Image(imgSrc);       // install new background

  unlock(&mutex);                    // release mutex
}
```
From the perspective of exception safety, this function is bad as it violates
* Leak no resources: if this throws before unlock, mutex is locked forever
* Don't allow data structures to become corrupted: if `new Image(imgSrc)` throws, bgImage is left pointing at a deleted object, and `imageChanges` is incremented

Item 14 introduces a lockguard like RAII object to tackle mutex locking forever.
```cpp
void PrettyMenu::changeBackground(std::istream& imgSrc)
{
  Lock ml(&mutex);                 // from Item 14: acquire mutex and
                                   // ensure its later release
  delete bgImage;
  ++imageChanges;
  bgImage = new Image(imgSrc);
}
```
To address data corruption, first we defines the terms.

Exception-safe functions offer one of the three guarantees:
* The basic guarantee promises if an exception is thrown, everything in the program is left in valid state. All objects are in internally consistent state, though the state of the entire program may not be predictable
* The strong guarantee promises if an exception is thrown, the state of the program is unchanged. Calls to such functions are atomic (transactional) in the sense that if they succeed they succeed completely, and if they fail it's like they've never been called
* The nothrow guarantee promises never to throw exceptions: the function always does what they promise to do. All operations on built-in types (int, pointer, etc) guarantee no throw. This is a critical building block of exception safe code.

Empty throw specification does not mean a function is not going to throw.
`int doSomething() throw();` indicates if an exception is thrown, it's a serious error and the "unexpected" function should be called. 

Exception safe must offer one of the three guarantees, the choice is then to decide which guarantee is practical for the code you write.
As a general rule, you want to offer the strongest guarantee that's practical.

Anything dynamically allocating memory (all STL containers) typically throw a `bad_alloc` if it cannot find enough memory to satisfy the allocation.
Offer no throw when you can, but most of the time, the choice is between strong exception guarantee and basic exception guarantee.

Now if we change the `bgImage` pointer to a smart pointer (good idea from resource management perspective as well as exception safety perspective), and increment `imageChanges` after the `reset` (usually a good idea to change a status to reflect a state update only after the state update actually happens), we end up with
```cpp
class PrettyMenu {
  ...
  std::shared_ptr<Image> bgImage;
  ...
};

void PrettyMenu::changeBackground(std::istream& imgSrc)
{
  Lock ml(&mutex);

  bgImage.reset(new Image(imgSrc));  // replace bgImage's internal
                                     // pointer with the result of the
                                     // "new Image" expression
  ++imageChanges;
}
```
Now there is no need for manual delete, and delete only happens if the reset succeeds.

Now this is almost a strong guarantee, except that imgSrc marker might moved in case of an exception.

It's important to know a general strategy that typically leads to exception safe code: copy-and-swap.
In principle, if you want to change something, make a copy of it, change the copy, and swap the original with the copy.
If any of the operation throws, the original object is not affected.
And after the operations are done, use a no-throw swap to swap the copy and the original.

This is usually implemented with a pimpl. Like this
```cpp
struct PMImpl {                               // PMImpl = "PrettyMenu
  std::tr1::shared_ptr<Image> bgImage;        // Impl."; see below for
  int imageChanges;                           // why it's a struct
};

class PrettyMenu {
  ...

private:
  Mutex mutex;
  std::tr1::shared_ptr<PMImpl> pImpl;
};

void PrettyMenu::changeBackground(std::istream& imgSrc)
{
  using std::swap;                            // see Item 25

  Lock ml(&mutex);                            // acquire the mutex

  std::tr1::shared_ptr<PMImpl>                // copy obj. data
    pNew(new PMImpl(*pImpl));

  pNew->bgImage.reset(new Image(imgSrc));     // modify the copy
  ++pNew->imageChanges;

  swap(pImpl, pNew);                          // swap the new
                                              // data into place

}                                             // release the mutex
```
Copy-and-swap is excellent in making all or nothing changes to an object, though it doesn't guarantee strong exception safety.
Say if you make some calls inside `changeBackground`, `changeBackground` will only be as exception safe as those calls.

Say `changeBackground` makes two calls `f1` and `f2`, even if both offer strong exception safe guarantee, `changeBackground` may not. (`f1` modifies some states, `f2` then throws, states modified in `f1` will not be rolled back.)

The problem is side effect: if a function is side-effect (say, update a DB) free or operate only on local data, then it's easy to guarantee strong exception safety.
Otherwise it'll be hard, there is no general way to undo a DB operation, considering other clients might have updated it in between.

Another issue with copy-and-swap is efficiency: you have to make a copy, a cost you may not want to pay.

Strong exception safety is desirable, but you should offer it only when practical.
When it's not, you'll have to offer the basic guarantee.
Things are different if you offer no exception safety guarantee, in which case it's like guilty until proven innocent.
All of its callers would be unable to offer exception safety guarantee, and the system in turn offers no exception safe guarantee.

A function's exception-safety guarantee is a visible part of its interface, so you should choose it as deliberately as you choose all other aspects of a function's interface.

Don't use interaction with legacy code as your excuse to not write exception safe code.
Forty years ago, goto-laden code was considered perfectly good practice. Now we strive to write structured control flows. Twenty years ago, globally accessible data was considered perfectly good practice. Now we strive to encapsulate data. Ten years ago, writing functions without thinking about the impact of exceptions was considered perfectly good practice. Now we strive to write exception-safe code.

Time goes on. We live. We learn.

**Takeaways**
* Exception-safe functions leak no resources and allow no data structures to become corrupted, even when exceptions are thrown. Such functions offer the basic, strong, or nothrow guarantees
* The strong guarantee can often be implemented via copy-and-swap, but the strong guarantee is not practical for all functions
* A function can usually offer a guarantee no stronger than the weakest guarantee of the functions it calls

### Understand the ins and outs of inlining

Inline replaces a function call with the body of the function code, which saves function call overhead, but results in larger object code size (in turn, additional page, reduced instruction cache hits, etc).

On the other hand, if an inline function is very short, the code generated for the function body may be smaller than that generated for a function call, and the effects of larger object size would be reversed.

Bear in mind `inline` is a request to the compiler, not a command.
Inline can be implicit or explicit:

```cpp
// implicit: defining a function inside a class definition
class Person {
public:
  ...
  int age() const { return theAge; }    // an implicit inline request: age is
  ...                                   // defined in a class definition
private:
  int theAge;
};

// explicit: (e.g. the definition of std::max)
template<typename T>                               // an explicit inline
inline const T& std::max(const T& a, const T& b)   // request: std::max is
{ return a < b ? b : a; }                          // preceded by "inline"
```

Inline functions must typically be in header files, because most compilers do inline during compilation, compilers need to know what the function looks like in order to inline.
(Some compilers can inline at linking or even at runtime (.Net), but inlining in C++ is mostly compile time)

Templates are typically in header files, because compiler needs to know what a template looks like in order to instantiate it.
(Some compilers can do instantiation at linking)

It's not true that function templates must be inline, they are independent.

Most compilers refuse to inline a function deemed too complicated (e.g. loops or recursion)

Virtual function calls cannot be inlined: if what to call is only known at runtime, then compiler cannot replace the function call with function body.

What's inlined would end up depending on the compiler, who usually emits a warning when it refuses to inline something you told it to.

If your program takes the address of a function, compiler will need to generate a function body anyway.
Compilers generally don't perform inlining over calls made through function pointers.
Thus if something ends up being inlined or not sometimes depends on how it's called. Like
```cpp
inline void f() {...}      // assume compilers are willing to inline calls to f

void (*pf)() = f;          // pf points to f

...

f();                      // this call will be inlined, because it's a "normal" call

pf();                     // this call probably won't be, because it's through
                          // a function pointer
```

Even if you don't use function pointers, compilers may do.
In fact, ctors and dtors are often worse candidates for inlining than a casual examination would indicate.
For example,
```cpp
class Base {
public:
...

private:
   std::string bm1, bm2;               // base members 1 and 2
};

class Derived: public Base {
public:
  Derived() {}                         // Derived's ctor is empty — or is it?
  ...

private:
  std::string dm1, dm2, dm3;           // derived members 1–3
};
```
`Derived`'s ctor may contain no user code, but in order to make Base class construction happen, `string` members construction happen, and rollback if a part of it is done then exception happens, compiler has to generate code.
E.g. like the following
```cpp
Derived::Derived()                       // conceptual implementation of
{                                        // "empty" Derived ctor

Base::Base();                           // initialize Base part

try { dm1.std::string::string(); }      // try to construct dm1
catch (...) {                           // if it throws,
   Base::~Base();                        // destroy base class part and
   throw;                                // propagate the exception
}

try { dm2.std::string::string(); }      // try to construct dm2
catch(...) {                            // if it throws,
   dm1.std::string::~string();           // destroy dm1,
   Base::~Base();                        // destroy base class part, and
   throw;                                // propagate the exception
}

try { dm3.std::string::string(); }      // construct dm3
catch(...) {                            // if it throws,
   dm2.std::string::~string();           // destroy dm2,
   dm1.std::string::~string();           // destroy dm1,
   Base::~Base();                        // destroy base class part, and
   throw;                                // propagate the exception
}
}
```
Considering all the code added by the compiler, inlining a ctor or dtor might lead to excessive bloat in object code.

Library compiler must also consider that inlining a function makes it get compiled with client's code.
Should the library decide to change that function, clients would now need to recompile as opposed to relink, which is often times undesirable.
And if the library is dynamically, this change may be absorbed in a way transprent to clients.

Most debuggers have trouble with inline functions (how do you set a breakpoint to a function that's not there?).
Some debuggers do support it, while others may simply disable inlining for debug builds.

To summarize the strategy with regard to inlining, initially don't inline anything (or limit to those that have to be inline or trivial).
Then figure out the right functions to inline. (Remember the 80-20 rule of 80% of time might be spent executing 20% of the code, thus finding out the right functions to inline is important)

**Takeaways**
* Limit most inlining to small, frequently called functions. This facilitates debugging and binary upgradability, minimizes potential code bloat, and maximizes the chances of greater program speed
* Don't declare function templates inline just because they appear in header files

### Minimize compilation dependencies between files


