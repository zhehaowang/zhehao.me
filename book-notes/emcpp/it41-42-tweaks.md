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

