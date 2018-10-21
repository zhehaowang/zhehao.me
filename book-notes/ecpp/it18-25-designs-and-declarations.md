# Designs and declarations

Guidelines on designing good interfaces.

### Make interfaces easy to use correctly and hard to use incorrectly

Assuming your clients want to use your interface correctly, then if they manage to use it incorrectly, your interface would be partially to blame.

Ideally, if an attempted use of an interface won't do what the client expects, the code won't compile; and if the code does compile, it will do what the client wants.

Consider this interface
```cpp
class Date {
public:
  Date(int month, int day, int year);
  ...
};
```
What could go wrong? Clients may pass parameters in wrong order, or simply keyed in the wrong int (meh).

Many client errors can be avoided with the introduction of new types: the type system is your primary ally in preventing undesirable code from compiling.

In this case, say we have this
```cpp
struct Day {            struct Month {                struct Year {
  explicit Day(int d)     explicit Month(int m)         explicit Year(int y)
  :val(d) {}              :val(m) {}                    :val(y){}

  int val;                int val;                      int val;
};                      };                            };
class Date {
public:
Date(const Month& m, const Day& d, const Year& y);
...
};
Date d(30, 3, 1995);                      // error! wrong types

Date d(Day(30), Month(3), Year(1995));    // error! wrong types

Date d(Month(3), Day(30), Year(1995));    // okay, types are correct
```

Making Day, Month, and Year full-fledged classes with encapsulated data would be better than the simple use of structs above (see Item 22).

Once the right types are in place, it can sometimes be reasonable to restrict the values of those types.
For example, there are only 12 valid month values, so the Month type should reflect that.

One way to achieve that is to use enums, with C++11, enum classes due to type-safety.
Without using enums, you could do the following.

```cpp
class Month {
public:
  static Month Jan() { return Month(1); }   // functions returning all valid
  static Month Feb() { return Month(2); }   // Month values; see below for
  ...                                       // why these are functions, not
  static Month Dec() { return Month(12); }  // objects
  
  ...                                       // other member functions

private:
  explicit Month(int m);                    // prevent creation of new
                                            // Month values

  ...                                       // month-specific data
};
Date d(Month::Mar(), Day(30), Year(1995));
```
If the idea of using functions instead of objects to represent specific months strikes you as odd, it may be because you have forgotten that reliable initialization of non-local static objects can be problematic. Item 4 can refresh your memory.

And to remind you, unless there is good reason not to, have your types behave consistently with the built-in types. 
The real reason can be phrased as have interfaces behave consistently (with built-in types, amongst themselves.)
Think the STL: their container interfaces are largely (though not perfectly) consistent, and this helps make them fairly easy to use.
E.g. every STL container has a member function named size that tells how many objects are in the container.

Another way to prevent client errors is to restrict what can be done with a type, a common way is to add const qualifier.

Any interface that requires that clients remember to do something is prone to incorrect use, because clients can forget to do it.
For example this one,
```cpp
Investment* createInvestment();   // from Item 13; parameters omitted
                                  // for simplicity
```
Returning a raw pointer means the client needs to remember to delete that pointer exactly once, when they are done using it.
This would be error prone.
Instead, the interface could return a std::unique\_ptr or a std::shared\_ptr, which also has the benefit if you need a custom deleter behavior (instead of relying on client calling a deleteInvestment(), you could bind that in the smart pointer instantiation).
Like this (with custom deleter)
```cpp
std::tr1::shared_ptr<Investment> createInvestment()
{
  std::tr1::shared_ptr<Investment> retVal(nullptr,
                                          getRidOfInvestment);

  ...                                    // make retVal point to the
                                         // correct object

  return retVal;
}
```
The shared\_ptr version also works around the issue of cross-DLL deletion, where an object is created using new in one DLL and freed in another one. The shared pointer deletion in this case would guarantee that the same DLL news and deletes this object.

**Takeaways**
* Good interfaces are easy to use correctly and hard to use incorrectly. Your should strive for these characteristics in all your interfaces.
* Ways to facilitate correct use include consistency in interfaces and behavioral compatibility with built-in types.
* Ways to prevent errors include creating new types, restricting operations on types, constraining object values, and eliminating client resource management responsibilities.
* std::shared\_ptr supports custom deleters. This prevents the cross-DLL problem, can be used to automatically unlock mutexes (see Item 14), etc.

### Treat class design like type design

Defining a new class meant designing a new type, meaning you're not just a class designer, you're a type designer augmenting C++'s type system.
You should therefore approach class design with the same care that language designers lavish on the design of the language's built-in types.
Good types have a natural syntax, intuitive semantics, and one or more efficient implementations.

How, then, do you design effective classes? First, you must understand the issues you face.
* **How should objects of your new type be created and destroyed**: this affects your ctor and dtor, and potentially operator new delete new[] delete[] overload
* **How should object initialization differ from object assignment**: this lets you decide how your ctors differ from assignment oprs
* **What does it mean for objects of your new type to be passed by value**: copy ctor defines how pass-by-value is implemented for a type
* **What are the restrictions on legal values for your new type**: usually only certain combinations of values for data members are valid (invariants that your class has to maintain). This determines the error checking you need to do, and your exception specification
* **Does your new type fit into an inheritance graph**: do you inherit from other classes, what functions of theirs are declared virtual, do you intend for other classes to inherit from yours, what functions should your class declare virtual
* **What kind of conversions are allowed for your type**: if you wish T1 to be implicitly convertible to T2, you will want either an operator T2 inside T1, or an non-explicit ctor in T2 that takes T1. If you want explicit conversions only, you'll write the conversions but avoid the two implicit approaches
* **What operators and functions make sense for the new type**
* **What standard functions should be disallowed**
* **Who should have access to the members of your new type**: public, protected, private members; friend functions; nest one class in another?
* **What is the "undeclared interface" of your new type**: what guarantees do your class offer in terms of performance, exception safety, and resource usage (think locks and dynamic memory)
* **How general is your new type**: are you defining one new type or a family of types? If it's the latter, you should probably define a class template
* **Is the new type really what you need**

**Takeaways**
* Class design is type design. Before defining a new type, be sure to consider all the issues discussed in this item.

### Prefer pass-by-reference-to-const to pass-by-value

By default C++ passes by value, function parameters are initialized with copies of the actual argument.
These copies are produced by the objects' copy ctors.
This can make pass-by-value expensive.

Passing parameters by reference also avoids the slicing problem: when a derived class object is passed by value as a base class object, the part that only belongs to the derived part will be sliced off (since only a base class copy ctor is called).
Like passing by pointer, passing by reference does not have slicing problem.

If you peek under the hood of a C++ compiler, you'll find that references are typically implemented as pointers, so passing something by reference usually means really passing a pointer.
As a result, if you have an object of a built-in type (e.g., an int), it's often more efficient to pass it by value than by reference.
This same advice applies to iterators and function objects in the STL, because, by convention, they are designed to be passed by value. Implementers of iterators and function objects are responsible for seeing to it that they are efficient to copy and are not subject to the slicing problem.

Just because an object is small doesn't mean that calling its copy constructor is inexpensive: we pass built-in types by value not because they are small, but because of the underlying compiler impl (some compilers treat built-in types and (even small) user types differently: objects containing only a bare int will not be put into registers, while a bare int / pointer will).

Another reason why small user-defined types are not necessarily good pass-by-value candidates is that, being user-defined, their size is subject to change.
They can get bigger in the next release, or change as you switch to a different C++ impl (like some's impl of std::string can be 7 times as big as others)

**Takeaways**
* Prefer pass-by-reference-to-const over pass-by-value. It's typically more efficient and it avoids the slicing problem
* The rule doesn't apply to built-in types and STL iterator and function object types. For them, pass-by-value is usually appropriate

### Don't try to return a reference when you must return an object

Once folks learn passing by reference, some become so relenting with it that they start passing references to things that don't exist.

Consider this
```cpp
class Rational {
public:
  Rational(int numerator = 0,               // see Item 24 for why this
           int denominator = 1);            // ctor isn't declared explicit

  ...

private:
  int n, d;                                 // numerator and denominator

friend
   const Rational&                           // see Item 3 for why the
     operator*(const Rational& lhs,         // return type is const
               const Rational& rhs) {
     Rational result(lhs.n * rhs.n, lhs.d * rhs.d);
     return result;
   }
};
```
Returning a reference to a local stack-allocated object would bring undefined behavior to anyone calling operator\*.

What about a heap allocated version? It won't be undefined behavior, but who's in charge of calling delete?

Imagine it's heap allocated with new instead and this code
```cpp
Rational w, x, y, z;

w = x * y * z;                     // same as operator*(operator*(x, y), z)
```
Here is a guaranteed leak as two objects will be heap allocated, but there is only reference to one of them.

What about an even more exotic approach using static?
```cpp
const Rational& operator*(const Rational& lhs,    // warning! yet more
                          const Rational& rhs)    // bad code!
{
  static Rational result;             // static object to which a
                                      // reference will be returned

  result = ...;                      // multiply lhs by rhs and put the
                                      // product inside result
  return result;
}

// client code
bool operator==(const Rational& lhs,            // an operator==
                const Rational& rhs);           // for Rationals

Rational a, b, c, d;

...
if ((a * b) == (c * d))  {
    do whatever's appropriate when the products are equal;
} else    {
   do whatever's appropriate when they're not;
}

// think of the equality test as
if (operator==(operator*(a, b), operator*(c, d)))
```
Not to mention the potentially undesirable lifetime, thread-safety issue of static, the above code's check will always be true.
The two operator\* calls would be returning reference to the same object, so they are always equal.

The right way to write a function that must return a new object is to have that function return a new object. For Rational's operator\*, that means either the following code or something essentially equivalent:
```cpp
inline const Rational operator*(const Rational& lhs, const Rational& rhs) {
  return Rational(lhs.n * rhs.n, lhs.d * rhs.d);
}
```

It all boils down to this: when deciding between returning a reference and returning an object, your job is to make the choice that offers correct behavior. Let your compiler vendors wrestle with figuring out how to make that choice as inexpensive as possible.

**Takeaways**
* Never return a pointer or reference to a local stack object, a reference to a heap-allocated object, or a pointer or reference to a local static object if there is a chance that more than one such object will be needed. (Item 4 provides an example of a design where returning a reference to a local static is reasonable, at least in single-threaded environments.)

### Declare data members private

Why not public data members?
* Syntactic consistency (item 18), clients will know always to retrieve data members with getter functions instead of scratching their head trying to remember.
* Member functions grant you more precise control. With member functions you can control read / write access, but with public members you can't.
* Most importantly, encapsulation. If you implement access to a data member with a function, you can later replace the data member with a computation, and no clients will be affected. Hiding data members behind functional interfaces can offer all kinds of implementation flexibility. E.g. it makes it easy to notify other objects when data members are read or written, to verify class invariants and function pre-and postconditions, to perform synchronization in threaded environments, etc.

If you hide your data members from your clients (i.e., encapsulate them), you can ensure that class invariants are always maintained, because only member functions can affect them. Furthermore, you reserve the right to change your implementation decisions later.
Public means unencapsulated, and practically speaking, unencapsulated means unchangeable, especially for classes that are widely used.

The argument against protected data members is similar.
Aren't protected data members more encapsulated than public ones? Practically speaking, the surprising answer is that they are not.
Something's encapsulation is inversely proportional to the amount of code that might be broken if that something changes.

Suppose we have a protected data member, and we eliminate it.
How much code might be broken now? All the derived classes that use it, which is typically an unknowably large amount of code, not unlike the case with public data members.

From an encapsulation point of view, there are really only two access levels: private (which offers encapsulation) and everything else (which doesn't).

**Takeaways**
* Declare data members private. It gives clients syntactically uniform access to data, affords fine-grained access control, allows invariants to be enforced, and offers class authors implementation flexibility
* protected is no more encapsulated than public

### Prefer non-member non-friend functions to member functions

