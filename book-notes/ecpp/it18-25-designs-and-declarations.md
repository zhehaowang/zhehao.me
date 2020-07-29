# Designs and declarations

Guidelines on designing good interfaces.

### Item 18. Make interfaces easy to use correctly and hard to use incorrectly

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

Making `Day`, `Month`, and `Year` full-fledged classes with encapsulated data would be better than the simple use of structs above (see Item 22).

Once the right types are in place, it can sometimes be reasonable to restrict the values of those types.
For example, there are only 12 valid month values, so the `Month` type should reflect that.

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
E.g. every STL container has a member function named `size` that tells how many objects are in the container.

Another way to prevent client errors is to restrict what can be done with a type, a common way is to add `const` qualifier.

Any interface that requires that clients remember to do something is prone to incorrect use, because clients can forget to do it.
For example this one,
```cpp
Investment* createInvestment();   // from Item 13; parameters omitted
                                  // for simplicity
```
Returning a raw pointer means the client needs to remember to delete that pointer exactly once, when they are done using it.
This would be error prone.

Instead, the interface could return a `std::unique_ptr` or a `std::shared_ptr`, which also has the benefit if you need a custom deleter behavior (instead of relying on client calling a `deleteInvestment()`, you could bind that in the smart pointer instantiation).

Like this (with custom deleter)
```cpp
std::shared_ptr<Investment> createInvestment()
{
  std::shared_ptr<Investment> retVal(nullptr,
                                     getRidOfInvestment);

  ...                                    // make retVal point to the
                                         // correct object

  return retVal;
}
```
The `shared_ptr` version also works around the issue of cross-DLL deletion, where an object is created using `new` in one DLL and freed in another one. The shared pointer deletion in this case would guarantee that the same DLL `new`s and `delete`s this object.

**Takeaways**
* Good interfaces are easy to use correctly and hard to use incorrectly. Your should strive for these characteristics in all your interfaces.
* Ways to facilitate correct use include consistency in interfaces and behavioral compatibility with built-in types.
* Ways to prevent errors include creating new types, restricting operations on types, constraining object values, and eliminating client resource management responsibilities.
* `std::shared_ptr` supports custom deleters. This prevents the cross-DLL problem, can be used to automatically unlock mutexes (see Item 14), etc.

### Item 19. Treat class design like type design

Defining a new class meant designing a new type, meaning you're not just a class designer, you're a type designer augmenting C++'s type system.
You should therefore approach class design with the same care that language designers lavish on the design of the language's built-in types.
Good types have a natural syntax, intuitive semantics, and one or more efficient implementations.

How do you design effective classes? First, you must understand the issues you face.
* **How should objects of your new type be created and destroyed**: this affects your ctor and dtor, and potentially operator `new` `delete` `new[]` `delete[]` overload
* **How should object initialization differ from object assignment**: this lets you decide how your ctors differ from assignment oprs
* **What does it mean for objects of your new type to be passed by value**: copy ctor defines how pass-by-value is implemented for a type
* **What are the restrictions on legal values for your new type**: usually only certain combinations of values for data members are valid (invariants that your class has to maintain). This determines the error checking you need to do, and your exception specification
* **Does your new type fit into an inheritance graph**: do you inherit from other classes, what functions of theirs are declared `virtual`, do you intend for other classes to inherit from yours, what functions should your class declare `virtual`
* **What kind of conversions are allowed for your type**: if you wish `T1` to be implicitly convertible to `T2`, you will want either an `operator T2` inside `T1`, or an non-explicit ctor in `T2` that takes `T1`. If you want explicit conversions only, you'll write the conversions but avoid the two implicit approaches
* **What operators and functions make sense for the new type**
* **What standard functions should be disallowed**
* **Who should have access to the members of your new type**: `public`, `protected`, `private` members; `friend` functions; nest one class in another?
* **What is the "undeclared interface" of your new type**: what guarantees do your class offer in terms of performance, exception safety, and resource usage (think locks and dynamic memory)
* **How general is your new type**: are you defining one new type or a family of types? If it's the latter, you should probably define a class template
* **Is the new type really what you need**

**Takeaways**
* Class design is type design. Before defining a new type, be sure to consider all the issues discussed in this item.

### Item 20. Prefer pass-by-reference-to-const to pass-by-value

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

### Item 21. Don't try to return a reference when you must return an object

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
     operator*(const Rational& lhs,          // return type is const
               const Rational& rhs) {
     Rational result(lhs.n * rhs.n, lhs.d * rhs.d);
     return result;
   }
};
```
Returning a reference to a local stack-allocated object would bring undefined behavior to anyone calling `operator*`.

What about a heap allocated version? It won't be undefined behavior, but who's in charge of calling `delete`?

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

  result = ...;                       // multiply lhs by rhs and put the
                                      // product inside result
  return result;
}

// client code
bool operator==(const Rational& lhs,            // an operator==
                const Rational& rhs);           // for Rationals

Rational a, b, c, d;

...
if ((a * b) == (c * d))  {
   // do whatever's appropriate when the products are equal;
} else    {
   // do whatever's appropriate when they're not;
}

// think of the equality test as
if (operator==(operator*(a, b), operator*(c, d)))
```
Not to mention the potentially undesirable lifetime, thread-safety issue of `static`, the above code's check will always be true.
The two `operator*` calls would be returning reference to the same object, so they are always equal.

The right way to write a function that must return a new object is to have that function return a new object. For `Rational::operator*`, that means either the following code or something essentially equivalent:
```cpp
inline const Rational operator*(const Rational& lhs, const Rational& rhs) {
  return Rational(lhs.n * rhs.n, lhs.d * rhs.d);
}
```

It all boils down to this: when deciding between returning a reference and returning an object, your job is to make the choice that offers correct behavior. Let your compiler vendors wrestle with figuring out how to make that choice as inexpensive as possible.

**Takeaways**
* Never return a pointer or reference to a local stack object, a reference to a heap-allocated object, or a pointer or reference to a local `static` object if there is a chance that more than one such object will be needed. (Item 4 provides an example of a design where returning a reference to a local `static` is reasonable, at least in single-threaded environments.)

### Item 22. Declare data members `private`

Why not `public` data members?
* Syntactic consistency (item 18), clients will know always to retrieve data members with getter functions instead of scratching their head trying to remember.
* Member functions grant you more precise control. With member functions you can control read / write access, but with public members you can't.
* Most importantly, encapsulation. If you implement access to a data member with a function, you can later replace the data member with a computation, and no clients will be affected. Hiding data members behind functional interfaces can offer all kinds of implementation flexibility. E.g. it makes it easy to notify other objects when data members are read or written, to verify class invariants and function pre-and post-conditions, to perform synchronization in threaded environments, etc.

If you hide your data members from your clients (i.e., encapsulate them), you can ensure that class invariants are always maintained, because only member functions can affect them. Furthermore, you reserve the right to change your implementation decisions later.
Public means unencapsulated, and practically speaking, unencapsulated means unchangeable, especially for classes that are widely used.

The argument against `protected` data members is similar.
Aren't `protected` data members more encapsulated than public ones? Practically speaking, the surprising answer is that they are not.
Something's encapsulation is inversely proportional to the amount of code that might be broken if that something changes.

Suppose we have a `protected` data member, and we eliminate it.
How much code might be broken now? All the derived classes that use it, which is typically an unknowably large amount of code, not unlike the case with public data members.

From an encapsulation point of view, there are really only two access levels: `private` (which offers encapsulation) and everything else (which doesn't).

**Takeaways**
* Declare data members `private`. It gives clients syntactically uniform access to data, affords fine-grained access control, allows invariants to be enforced, and offers class authors implementation flexibility
* `protected` is no more encapsulated than `public`

### Item 23. Prefer non-member non-friend functions to member functions

Often times you'll find yourself facing the choice of having a function being a member of a class, a function in this translation unit / namespace.
Say, you have an object `o` with member functions `a`, `b`, `c`, and there is an action `abc()` that calls `o.a()`, `o.b()`, `o.c()`. Should `abc()` be a part of the class, or not (say, being a part of the namespace that the class is in)?

Object-oriented principles dictate that data and the functions that operate on them should be bundled together.
Object-oriented principles dictate that data should be as encapsulated as possible.

Start by inspecting encapsulation. Encapsulating something means it's hidden from view. The more something is encapsulated, the fewer see it, and the greater flexibility we have to change it.
This is why we value encapsulation: to be able to change something in a way that only affects a limited number of clients.
How encapsulated a data member in a class is can be evaluated by how many functions can access it. The less the number of functions accessing it, the more encapsulated it is.

Thus when given the choice of a member / friend function vs a non-member non-friend option, the preferred choice in terms of encapsulation is always the non-member non-friend function.

C++ doesn't require that all functions be a part of a class as Java, C\# does, so a natural approach in this case is to make the function `(abc())` a part of the same namespace that the class is in.

Namespace, unlike classes, can spread across multiple files, and often times it only makes sense for some clients to know this `abc()`, and for those who don't care their compilation shouldn't require the declaration of `abc()` at all.
To address this, we could split these functions declarations into different headers.
This is how the `std` namespace is organized. memory, list, algorithm, vector, etc.
Clients only need to include part of the std library headers where the required symbol is declared, and in turn their compilation would only depend on those headers.

Partioning into different headers like described above is not possible for class member functions, as they have to appear in one file.

This approach of putting `abc()` in the namespace of the class also allows clients to easily extend the namespace with helper functions they need. This is another feature the member function approach cannot offer: classes are closed to extension by clients.

**Takeaways**
* Prefer non-member non-friend functions to member functions. Doing so increases encapsulation, packaging flexibility, and functional extensibilit

### Declare non-member functions when type conversions should apply to all parameters

Having classes support implicit conversions is generally a bad idea, but there are exceptions. For example, a numerical Rational class.
Having int and float being able to implicit convert to Rational is not a bad idea.

You may have this
```cpp
class Rational {
public:
  Rational(int numerator = 0,        // ctor is deliberately not explicit;
           int denominator = 1);     // allows implicit int-to-Rational
                                     // conversions

  int numerator() const;             // accessors for numerator and
  int denominator() const;           // denominator — see Item 22

private:
  ...
};
```

You know you'd like to support arithmetic operations like addition, multiplication, etc., but how?
Which one to choose among member functions, non-member functions, or non-member functions that are friends?

Say you go with member functions
```cpp
class Rational {
public:
...

const Rational operator*(const Rational& rhs) const;
};
```

This is fine
```cpp
Rational oneEighth(1, 8);
Rational oneHalf(1, 2);

Rational result = oneHalf * oneEighth;            // fine

result = result * oneEighth;                      // fine
```

But if you also want to support doing multiply with an int, this breaks
```cpp
result = oneHalf * 2;                             // fine
result = 2 * oneHalf;                             // error!
// or if you rewrite the two, it becomes more obvious
result = oneHalf.operator*(2);                    // fine, implicit conversion from 2 to Rational
                                                  // doable because Rational ctor is not explicit
result = 2.operator*(oneHalf);                    // error!
```

oneHalf has an operator\*, so it's fine.
int doesn't, so compiler will look for non-member functions that can be called like operator\*(2, oneHalf), i.e. functions that are global, or in namespaces. But in this example, that search also fails.

It turns out that parameters are eligible for implicit conversion only if they are listed in the parameter list.
In terms of member function, \*this is not eligible to become target of implicit conversion, causing the second statement to fail.

So, to support mixed mode operators consistently, one approach is to make operator\* not a member. Like this
```cpp
const Rational operator*(const Rational& lhs,     // now a non-member
                         const Rational& rhs)     // function
{
  return Rational(lhs.numerator() * rhs.numerator(),
                  lhs.denominator() * rhs.denominator());
}

Rational oneFourth(1, 4);
Rational result;

result = oneFourth * 2;                           // fine
result = 2 * oneFourth;                           // hooray, it works!
```

The next question is should operator\* be a friend of Rational?
In this case no, because operator\* can be implemented entirely on Rational's public interface.
Whenever you can avoid friend functions, you should.

This item contains nothing but truth, but not the whole truth.
When Rational is class template instead of a class, there are new things to consider.

**Takeaways**
* If you need type conversions on all parameters to a function (including the one pointed to by the this pointer), the function must be a non-member

### Consider support for a non-throwing swap

swap is originally included as part of STL, and has since been a mainstay of exception-safe programming, and used to cope with the possibility of assignment to self.

A typical implementation of std::swap is like
```cpp
namespace std {
  template<typename T>          // typical implementation of std::swap;
  void swap(T& a, T& b)         // swaps a's and b's values
  {
    T temp(a);
    a = b;
    b = temp;
  }
}
```
As long as your type supports copycon and copy assignment opr, the std::swap will work without additional effort.

But this default implementation can be slow: three copy calls.
E.g. think of a class following pimpl idiom (item 31):
```cpp
class WidgetImpl {                          // class for Widget data;
public:                                     // details are unimportant
  ...

private:
  int a, b, c;                              // possibly lots of data —
  std::vector<double> v;                    // expensive to copy!
  ...
};

class Widget {                              // class using the pimpl idiom
public:
  Widget(const Widget& rhs);

  Widget& operator=(const Widget& rhs)      // to copy a Widget, copy its
  {                                         // WidgetImpl object. For
   ...                                      // details on implementing
   *pImpl = *(rhs.pImpl);                   // operator= in general,
   ...                                      // see Items 10, 11, and 12.
  }
  ...

private:
  WidgetImpl *pImpl;                         // ptr to object with this
};                                           // Widget's data
```
To swap two Widget objects we only need to swap the pointers, yet the std::swap has no way of knowing that.

Using total template specialization (template\<\>, with the later \<Widget\> to specify this specialization is for Widget), we could tell std::swap function template to swap only the implementation pointers when dealing with Widget objects:
```cpp
namespace std {
  template<>                            // this is a specialized version
  void swap<Widget>(Widget& a,          // of std::swap for when T is
                    Widget& b)          // Widget; this won't compile
  {
    swap(a.pImpl, b.pImpl);             // to swap Widgets, just swap
  }                                     // their pImpl pointers
}
```
This, however, wouldn't compile due to pImpl being private.
We could have this swap being a friend, but the convention here is different, we have swap being a public member of Widget which calls std::swap to swap the pointers. Like this
```cpp
class Widget {                     // same as above, except for the
public:                            // addition of the swap mem func
  ...
  void swap(Widget& other)
  {
    using std::swap;               // the need for this declaration
                                   // is explained later in this Item

    swap(pImpl, other.pImpl);      // to swap Widgets, swap their
  }                                // pImpl pointers
  ...
};

namespace std {

  template<>                       // revised specialization of
  void swap<Widget>(Widget& a,     // std::swap
                    Widget& b)
  {
    a.swap(b);                     // to swap Widgets, call their
  }                                // swap member function
}
```
This compiles and is compliant with how STL containers do it: public swap member function and template specialization for each container.

What if Widget and WidgetImpl are instead class templates instead of classes?
The swap member function inside is fine, but we can't partial specialize the swap function in std namespace like this
```cpp
namespace std {
  template<typename T>
  void swap<Widget<T> >(Widget<T>& a,      // error! illegal code!
                        Widget<T>& b)
  { a.swap(b); }
}
```
Due to C++ allowing partial specialization for classes but not functions.

When you want to partially specialize a function, what you do instead is simply add an overload.
```cpp
namespace std {

  template<typename T>             // an overloading of std::swap
  void swap(Widget<T>& a,          // (note the lack of "<...>" after
            Widget<T>& b)          // "swap"), but see below for
  { a.swap(b); }                   // why this isn't valid code
}
```
In general, overloading function templates is fine, but std is special:
It's ok to specialize templates in std, but not ok to add new templates (or classes or functions or anything else) to std.
(However programs that cross this line will compile, but this will be undefined behavior.)

Unfortunately, this overload template in std will be seen as adding functions to std, and this approach yields undefined behavior per above. 
So what do we do?
We still declare an overload, just not in std namespace.
Say Widget's in namespace WidgetStuff, we could do
```cpp
namespace WidgetStuff {
  ...                                     // templatized WidgetImpl, etc.

  template<typename T>                    // as before, including the swap
  class Widget { ... };                   // member function

  ...

  template<typename T>                    // non-member swap function;
  void swap(Widget<T>& a,                 // not part of the std namespace
            Widget<T>& b)                                         
  {
    a.swap(b);
  }
}
```

Now if code anywhere calls swap on two Widgets, the name lookup rule (argument dependent lookup, or Koenig lookup) will find this version in WidgetStuff, which is what we want.
(If you are not using namespaces the above would still work, but why clog everything in global namespace?)

Should we use this approach all the time then?
There is still a case for specializing std::swap. In fact, to have your class specific version of swap be called in as many places as possible, you need to write both a non-member version in the same namespace as your class, and a specialization of std::swap.

Now, from a client's perspective, you want to call swap
```cpp
template<typename T>
void doSomething(T& obj1, T& obj2)
{
  ...
  swap(obj1, obj2);
  ...
}
```
Which one do you want to call?
The one in std (exist)? Its specialization (may or may not exist)? A T-specific swap (may exist but not in std namespace)?
You'll want to call a T-specific one if it exists, but fall back to general version in std if not.

To achieve this, you do
```cpp
template<typename T>
void doSomething(T& obj1, T& obj2)
{
  using std::swap;           // make std::swap available in this function
  ...
  swap(obj1, obj2);          // call the best swap for objects of type T
  ...
}
```
With this, name lookup rules dictate swap will find T-specific swap at global scope or in the same namespace as type T (argument dependent lookup).
If no T-specific swap exists, compilers will use swap in std, thanks to the using statement that makes std::swap visible.
Even then, compiler would still prefer a specialization if there is one, over the general std::swap template.

Keep in mind the one thing you don't want to do is qualify the call, like
```cpp
std::swap(obj1, obj2);  // the wrong way to call swap
```

You'd force the compiler to consider only the std one and its specializations, eliminating the possibility of using a more T-specific one elsewhere.

Some programmers do call swap like the above, thus the need for swap's writers to provide the fully specialized version, to accommodate such clients. (such code is present even in std's implementation)

To summarize:
* if the default swap offers acceptable efficiency for your class, don't do anything
* if not, do the following: offer a public member function swap (that should never throw); offer a non-member swap in the same namespace as your class, have it call the member version; if you are writing a class (not a class template), specialize std::swap for your class and have it call the member version

Finally, if you are using swap as a client, be sure to do using std::swap and not qualify the swap call.

Now on exception safety, the most useful application of swap is to help classes offer strong exception safety guarantee.
This constraint (never throws) only applies on the member version as the default version uses copycon and copy assignment, both of which are allowed to throw in general.

When you write a custom version of swap, you are typically offering more than just an efficient way to swap values; you're also offering one that doesn't throw exceptions.

As a general rule, these two swap characteristics go hand in hand, because highly efficient swaps are almost always based on operations on built-in types (such as the pointers underlying the pimpl idiom), and operations on built-in types never throw exceptions.

**Takeaways**
* Provide a swap member function when std::swap would be inefficient for your type. Make sure your swap doesn't throw exceptions
* If you offer a member swap, also offer a non-member swap that calls the member. For classes (not templates), specialize std::swap, too
* When calling swap, employ a using declaration for std::swap, then call swap without namespace qualification
* It's fine to totally specialize std templates for user-defined types, but never try to add something completely new to std
