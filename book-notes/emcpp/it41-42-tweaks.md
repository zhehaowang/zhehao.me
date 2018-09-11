# Tweaks

### Consider pass-by-value for copyable parameters that are cheap to move and always copied

Some function parameters are intended to be copied. E.g.
```cpp
class Widget {
public:
  void addName(const std::string& newName)    // take lvalue;
  { names.push_back(newName); }               // copy it

  void addName(std::string&& newName)         // take rvalue;
  { names.push_back(std::move(newName)); }    // move it; see
  …                                           // Item 25 for use
                                              // of std::move
private:
  std::vector<std::string> names;
};
```
This works but it requires two functions to declare and implement, two copies to maintain, and two copies of binary code if not inlined by the compiler.

To get rid of two identical copies, an alternative is to use a universal reference.
```cpp
class Widget {
public:
  template<typename T>                          // take lvalues
  void addName(T&& newName)                     // and rvalues;
  {                                             // copy lvalues,
    names.push_back(std::forward<T>(newName));  // move rvalues;
  }                                             // see Item 25
                                                // for use of
  …                                             // std::forward

};
```
As a template, addName's impl typically needs to be in header file, and it yields several functions in the object code, since it's not just rvalue and lvalue versions but also instantiates differently for types that are convertible to std::string.
Also this can't handle clients passing argument types that can't be passed by universal references (item 30), and if client passes improper types the error message can be daunting (item 27).

It'd be nice to write one copy of addName, have one copy of addName in generated code, without using universal references, and does copy for lvalue and move for rvalues.
The solution could be
```cpp
class Widget {
public:
  void addName(std::string newName)           // take lvalue or
  { names.push_back(std::move(newName)); }    // rvalue; move it

  …

};
```
Typically move is used with rvalue references, but in this case we know newName is independent of what's passed in so changing it doesn't matter, and this is the final use of newName.

What about efficiency? This goes against the fundamental recommendation of passing user defined types by const reference.
In C++98 this will likely be slow, no matter what the user passes in a copy ctor is invoked.
In C++11, newName is copy ctor'ed only for lvalues and move ctor'ed for rvalues.
```cpp
Widget w;

…

std::string name("Bart");

w.addName(name);                 // call addName with lvalue
                                 // (copy ctor'ed)
…

w.addName(name + "Jenne");       // call addName with rvalue
                                 // (move ctor'ed)
```
This achieves just the desired effect of copy if lvalue, move if rvalue.

We then analyze the three approaches,
* in the rvalue and lvalue references approach, it's one copy for lvalues and one move for rvalues
* in the universal reference approach, it's one copy for lvalues and one move for rvalues. (if users pass in types that can be converted to std::string it can be as few as 0 copies / moves due to code generated for that particular type, item 25)
* in the pass by value approach, the parameter is copied anyway, so it's one copy + one move for lvalues, and two moves for rvalues. It's one more move than the first two approaches.

Thus the title of this item:
* you should consider passing by value for writing only one copy of the function, generating one copy of object code, but incurs additional cost of one move
* consider pass by value only for copiable parameters. In the move-only case, the first approach would only contain one signature taking in rvalue references in the first place. E.g. a std::unique\_ptr:
```cpp
class Widget {
public:
  …
  void setPtr(std::unique_ptr<std::string>&& ptr)
  { p = std::move(ptr); }

private:
  std::unique_ptr<std::string> p;
};

// a caller might use it this way
Widget w;
…

w.setPtr(std::make_unique<std::string>("Modern C++"));
// here the total cost is one move

// if Widget setPtr was defined instead as
class Widget {
public:
  …

  void setPtr(std::unique_ptr<std::string> ptr)
  { p = std::move(ptr); }

  …
};

// the cost would be move ctor'ing ptr then moving it to p
// thus the total cost of two moves
```
* Pass by value in this case is only worth considering for arguments that are cheap to move: according to the above analysis an extra move is incurred in the pass by value approach
* Consider pass by value for parameters that are always copied. Consider this
```cpp
class Widget {
public:
  void addName(std::string newName)
  {
    if ((newName.length() >= minLen) &&
        (newName.length() <= maxLen))
      {
        names.push_back(std::move(newName));
      }
  }

  …

private:
  std::vector<std::string> names;
};
```
If an lvalue is given, the function copies / moves anyway even if sometimes names do not have this copy moved in.

Even when you’re dealing with a function performing an unconditional copy on a copyable type that’s cheap to move, there are times when pass by value may not be appropriate.
That’s because a function can copy a parameter in two ways: via construction (i.e., copy construction or move construction) and via assignment (i.e., copy assignment or move assignment).
addName uses copy / move ctor, thus the analysis we saw above.

If it uses assignment instead, consider this code
```cpp
class Password {
public:
  explicit Password(std::string pwd)     // pass by value
  : text(std::move(pwd)) {}              // construct text

  void changeTo(std::string newPwd)      // pass by value
  { text = std::move(newPwd); }          // assign text

  …

private:
  std::string text;                      // text of password
};

// this is all good as analyzed above, one extra move ctor is all
std::string initPwd("Supercalifragilisticexpialidocious");

Password p(initPwd);

// this may cause the function to explode in cost
std::string newPassword = "Beware the Jabberwock";

p.changeTo(newPassword);
// reasoning being: one string copy ctor (and dynamic allocation)
// is invoked, and one dtor is invoked (to deallocate the current
// string in text): two dynamic allocations

// if instead we have the by reference approach
void changeTo(const std::string& newPwd)      // the overload
{                                             // for lvalues
  text = newPwd;           // can reuse text's memory if
                           // text.capacity() >= newPwd.size()
}
// no dynamic allocation is involved if text.capacity() >= newPwd.size()
// if not, it's typically not possible to work around the two dynamic
// allocations.

// this kind of analysis applies to types that involves dynamic allocation
// like std::string and std::vector
// whether the impl uses small string optimization also matters for the
// analysis
```

And there are other issues with passing by reference: what if you have a chain of calls and many functions like addName, and each of them incurs an extra move? What if you have a function that is designed to accept a parameter of a base class type or any type derived from it (passing by value would cause slicing off)? 

Usually, the most practical approach is to adopt a "guilty until proven innocent" policy, whereby you use overloading or universal references instead of pass by value unless it’s been demonstrated that pass by value yields acceptably efficient code for the parameter type you need.

**Takeaways**
* For copyable, cheap-to-move parameters that are always copied, pass by value may be nearly as efficient as pass by reference, it’s easier to implement, and it can generate less object code.
* For lvalue arguments, pass by value (i.e., copy construction) followed by move assignment may be significantly more expensive than pass by reference followed by copy assignment.
* Pass by value is subject to the slicing problem, so it’s typically inappropriate for base class parameter types.

### Consider emplacement instead of insertion

When you have a container holding std::string, it seems logical when you add an element to it you add via insertion functions (insert, push\_front, push\_back, insert\_after), and the type you'd pass in is std::string.

Consider this code though:
```cpp
std::vector<std::string> vs;         // container of std::string

vs.push_back("xyzzy");               // add string literal
```

push\_back is overloaded as following:
```cpp
template <class T,                           // from the C++11
          class Allocator = allocator<T>>    // Standard
class vector {
public:
  …
  void push_back(const T& x);                // insert lvalue
  void push_back(T&& x);                     // insert rvalue
  // why is just having an universal reference version not enough?
  …
};
```
In the call of push\_back with string literal, compiler sees a type mismatch (const char[6] with what vector's instantiated with: std::string), thus what it does becomes the following (create a temporary):
```cpp
vs.push_back(std::string("xyzzy"));  // create temp. std::string
                                     // and pass it to push_back
// here std::string ctor is called, and then the rvalue reference
// is given to push_back to be move ctor'ed to the memory of the
// new object inside std::vector.
// immediately after push_back returns, the temp object is destroyed
// by calling std::string dtor.
```
Here the ctor and dtor of the temporary std::string object is avoidable: as we construct the new std::string object to be held inside the std::vector we could give it the string literal.
And emplace\_back does exactly that.
```cpp
vs.emplace_back("xyzzy");   // construct std::string inside
                            // vs directly from "xyzzy"
// no temporary involved
```
emplace\_back uses perfect forwarding, so it would have the failure / unexpected cases with perfect forwarding:
```cpp
vs.emplace_back(50, 'x');   // insert std::string consisting
                            // of 50 'x' characters
```
Every std container that supports insert supports a corresponding emplace as well.

Insertion functions take objects to be inserted, while emplacement functions take constructor arguments for objects to be inserted.
This difference permits emplacement functions to avoid the creation and destruction of temporary objects that insertion functions can necessitate.

emplacement function can be used even when an insertion function would require no temporary:
```cpp
std::string queenOfDisco("Donna Summer");
// these result in the same
vs.push_back(queenOfDisco);       // copy-construct queenOfDisco
                                  // at end of vs

vs.emplace_back(queenOfDisco);    // ditto
```

Why not always use emplacement then?
Because in current std library, there are scenarios where insertion is faster than emplacement.

Such scenarios are hard to categorize, but emplacement will almost certainly outperform insertion if all the following are true:
* The value being added is constructed into the container, not assigned. Consider this
```cpp
std::vector<std::string> vs;         // as before

…                                    // add elements to vs

vs.emplace(vs.begin(), "xyzzy");     // add "xyzzy" to
                                     // beginning of vs
```
The impl likely uses move assignment underneath, in which case a temporary object will need to be created to serve as the source of move.
Emplacement's edge would then disappear.
node based std containers always use ctor'ed elements instead of assigned, the rest (std::vector, std::string, std::deque, std::array (which is irrelevant)) you can rely on emplace\_back (and emplace\_front as well) to use ctor.
* The argument type(s) being passed differ from the type held by the container.
If they are the same insert would not create the temporary either.
* The container is unlikely to reject the new value as a duplicate.
Reason for this is that in order to detect if a value is already in the container, emplacement typically creates the node with the new value first so that it can compare the value with the rest.

Two other issues worth considering when using emplacement:

Suppose you have a container of std::shared\_ptrs:
```cpp
std::list<std::shared_ptr<Widget>> ptrs;

// say you have a custom deleter
void killWidget(Widget* pWidget);

// and you want to insert shared pointers with a custom deleter
ptrs.push_back(std::shared_ptr<Widget>(new Widget, killWidget));
// and it could look like
ptrs.push_back({ new Widget, killWidget });

// note that although recommended, you can't use make_shared here
// since a custom deleter is desired. 
```
With the push\_back approach, a temporary will be created.
Emplacement would avoid creating this temporary, but in this case the temporary is desirable: say during the allocation of the node in the list container an out-of-memory exception is thrown, then as the exception propagates out, the temp object will be freed and being the sole shared pointer referring to Widget object, Widget will be deallocated by calling killWidget.
Nothing leaks.

Now consider the emplacement version
```cpp
ptrs.emplace_back(new Widget, killWidget);

// you'd be calling something like this underneath
std::list::node<std::shared_ptr<Widget>>(new Widget, killWidget);

// think of the above as
template <typename T> 
class Node {
  template <typename... Ps>
  Node<T>(Ps&&... params) :
    something_other_data(xxx),
    T(std::forward<Ps>(params)) {}
  // what if it throws after "new Widget" is constructed, but
  // before T can be done allocated (say, some other member ctor
  // throws an out-of-memory)? As explained below, the new
  // Widget leaks.
};
```
The raw pointer resulting from "new Widget" is perfect forwarded to node ctor and if that ctor throws an exception, as the exception propagates out there is no handle to the heap allocated Widget any more.
It is leaked.

Similarly with std::unique\_ptr with a custom deleter.

Fundamentally, the effectiveness of memory management classes like std::unique\_ptr and std::shared\_ptr is predicated on resources (such as raw pointers from new) being immediately passed to ctors for resource managing objects.
The fact that make\_shared and make\_unique automate this is one of the reasons why they are important.

In cases like this, you need to ensure yourself you are not paying for potentially improved performance with diminished exception safety.

The emplacement / insert versions should then look more like this:
```cpp
// insert version
std::shared_ptr<Widget> spw(new Widget,    // create Widget and
                            killWidget);   // have spw manage it

ptrs.push_back(std::move(spw));            // add spw as rvalue

// emplacement version
std::shared_ptr<Widget> spw(new Widget, killWidget);
ptrs.emplace_back(std::move(spw));

// in which case emplacement won't outperform insert since spw is
// essentially the temporary now
```

The other case is with explicit ctors.

Say you wrote this by mistake,
```cpp
// using C++11's support for regex
std::vector<std::regex> regexes;

// you wrote this nullptr by mistake
regexes.emplace_back(nullptr);    // add nullptr to container
                                  // of regexes?
                                  // once compiled, this would
                                  // be UB.

// compiler does not reject this, even though
std::regex r = nullptr;           // error! won't compile
// or
regexes.push_back(nullptr);       // error! won't compile
```
This behavior stems from the fact that std::regex can be ctor'ed from character strings (const char \* like)
```cpp
std::regex upperCaseWord("[A-Z]+");
```
And this ctor taking a const char \* is explicit, thus the following
```cpp
std::regex r = nullptr;           // error! won't compile

regexes.push_back(nullptr);       // error! won't compile

std::regex r(nullptr);            // compiles, this is what
                                  // emplacement would
                                  // translate to, calling this
                                  // explicit ctor directly
```

In official terminologies,
```cpp
std::regex r1 = nullptr;         // error! won't compile
// called copy initialization, not eligible to use explicit ctor

std::regex r2(nullptr);          // compiles
// called direct initialization, eligible to use explicit ctor

// and thus
regexes.emplace_back(nullptr);  // compiles. Direct init permits
                                // use of explicit std::regex
                                // ctor taking a pointer

regexes.push_back(nullptr);     // error! copy init forbids
                                // use of that ctor
```
**Takeaways**
* In principle, emplacement functions should sometimes be more efficient than their insertion counterparts, and they should never be less efficient.
* In practice, they're most likely to be faster when (1) the value being added is constructed into the container, not assigned; (2) the argument type(s) passed differ from the type held by the container; and (3) the container won’t reject the value being added due to it being a duplicate.
* Emplacement functions may perform type conversions that would be rejected by insertion functions.

