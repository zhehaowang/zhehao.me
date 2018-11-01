# Inheritance and Object Oriented Design

### Make sure public inheritance models "is-a"

Public inheritance means "is-a", commit this to memory.

If `D` publicly inherits from `B`, then you are telling the compiler and every reader of your code that `D` is an instance of `B`, not vice versa.
`B` is a more general concept than `D`, and `D` is more specific than `B`.
Anywhere `B` can be used, a `D` can be used as well, because every object of type `D` is an object of type `B`.

Within the realm of C++, anything that expects an argument of type `B` (say, `B`, `const B&`, `B*`) a `D` would suffice.

This sounds simple but sometimes intuition can be misleading: a penguin is a bird, a bird can fly but a penguin cannot.

In this case, our language isn't precise.
To model that not all birds can fly and a penguin is one such bird, we can do
```cpp
class Bird {
  ...                                       // no fly function is declared
};

class FlyingBird: public Bird {
public:
  virtual void fly();
  ...
};

class Penguin: public Bird {
  ...                                       // no fly function is declared
};
```
(If your software does not model flying of a bird but rather just beaks and wings, then we don't need `FlyingBird`)

Say your software does concern flying, what about this
```cpp
void error(const std::string& msg);       // defined elsewhere

class Penguin: public Bird {
public:
  virtual void fly() { error("Attempt to make a penguin fly!");}

  ...
};
```
This can only be tested at runtime, and item 18 would suggest good interfaces prevent invalid code from compiling, so you should prefer rejecting at compile time as opposed to at runtime.

What about having a square publicly inherit from a rectangle? From geometry, sure, but consider this code
```cpp
class Rectangle {
public:
  virtual void setHeight(int newHeight);
  virtual void setWidth(int newWidth);

  virtual int height() const;               // return current values
  virtual int width() const;

  ...

};

void makeBigger(Rectangle& r)               // function to increase r's area
{
  int oldHeight = r.height();

  r.setWidth(r.width() + 10);               // add 10 to r's width

  assert(r.height() == oldHeight);          // assert that r's
}                                           // height is unchanged


class Square: public Rectangle {...};

Square s;

...

assert(s.width() == s.height());           // this must be true for all squares

makeBigger(s);                             // by inheritance, s is-a Rectangle,
                                           // so we can increase its area

assert(s.width() == s.height());           // this must still be true
                                           // for all squares
```
The issue here is that something applicable to a rectangle is not applicable to a square: changing its width independent of the height.
This means using public inheritance, which suggests everything applicable to a rectangle is applicable to a square, is not suitable for this case.

"Is-a" relationship is not the only one that can exist between classes.
Two other common relationships are "has-a" and "is-implemented-in-terms-of", which will be discussed in later items.

**Takeaways**
* Public inheritance means “is-a.” Everything that applies to base classes must also apply to derived classes, because every derived class object is a base class object

### Avoid hiding inherited names

Name hiding (shadowing) looks like this:
```cpp
int x;                        // global variable
void someFunc() {
  double x;                   // local variable

  std::cin >> x;              // read a new value for local x, hides global x
}
```

Now in an inheritance scenario,
```cpp
class Base {
private:
  int x;

public:
  virtual void mf1() = 0; // simple virtual
  virtual void mf2();     // simple virtual
  void mf3();             // non virtual

  ...
};

class Derived: public Base {
public:
  virtual void mf1();
  void mf4();

  ...
};
```
Note that we are talking about names here, that includes names of types (enums, nested classes and typedefs)

Suppose `mf4` is implemented like this
```cpp
void Derived::mf4() {
  ...
  mf2();
  ...
}
```
Compiler figures out what `mf2` refers to, they begin by looking at the local scope (that of `mf4`), then the containing scope, that of `Derived`, then the scope of the base class, and it finds `mf2`, so the search stops (if not, search continues to the namespace(s) containing `Base`, if any, and finally to the global scope).

Now suppose we have this
```cpp
class Base {
private:
  int x;

public:
  virtual void mf1() = 0;
  virtual void mf1(int);

  virtual void mf2();

  void mf3();
  void mf3(double);
  ...
};

class Derived: public Base {
public:
  virtual void mf1();
  void mf3();
  void mf4();
  ...
};

Derived d;
int x;

...
d.mf1();                   // fine, calls Derived::mf1
d.mf1(x);                  // error! Derived::mf1 hides Base::mf1

d.mf2();                   // fine, calls Base::mf2

d.mf3();                   // fine, calls Derived::mf3
d.mf3(x);                  // error! Derived::mf3 hides Base::mf3
```
From the perspective of name lookup, `Base::mf1` and `Base::mf3` are no longer inherited by `Derived`.
This applies even though the functions in the base and derived classes take different parameter types, and it also applies regardless of whether the functions are virtual or non-virtual.

The rationale behind this behavior is that it prevents you from accidentally inheriting overloads from distant base classes when you create a new derived class in a library or application framework.
In fact, if you are using public inheritance and you don't inherit the overloads, you are violating the is-a relationship between base and derived classes that Item 32 explains is fundamental to public inheritance.

That being the case, you'll almost always want to override C++'s default hiding of inherited names.
You do it with `using` declarations.
```cpp
class Base {
private:
  int x;

public:
  virtual void mf1() = 0;
  virtual void mf1(int);

  virtual void mf2();

  void mf3();
  void mf3(double);
  ...
};

class Derived: public Base {
public:
  using Base::mf1;       // make all things in Base named mf1 and mf3
  using Base::mf3;       // visible (and public) in Derived's scope

  virtual void mf1();
  void mf3();
  void mf4();
  ...
};
```
Now inheritance will work as expected
```cpp
Derived d;
int x;

...

d.mf1();                 // still fine, still calls Derived::mf1
d.mf1(x);                // now okay, calls Base::mf1

d.mf2();                 // still fine, still calls Base::mf2

d.mf3();                 // fine, calls Derived::mf3
d.mf3(x);                // now okay, calls Base::mf3
```
This means if you inherit from a base class with overloaded functions and you want to redefine or override only some of them, you need to include a `using` declaration for each name you'd otherwise be hiding.
If you don't, some of the names you'd like to inherit will be hidden.

It's conceivable that you sometimes won't want to inherit all the functions from your base class.
Under public inheritance, this should never be the case.
Under private inheritance, however, it can make sense.

Suppose `Derived` privately inherits from `Base`, and the only version of `mf1` `Derived` wants to inherit is the one taking no parameters.
`using` declaration won't do the trick here, as `using` makes all inherited functions with a given name visible in the derived class.
This is the case for a different technique: a simple forwarding function.
```cpp
class Base {
public:
  virtual void mf1() = 0;
  virtual void mf1(int);

  ...                                    // as before
};

class Derived: private Base {
public:
  virtual void mf1()                   // forwarding function; implicitly
  { Base::mf1(); }                     // inline (see Item 30)
  ...
};

Derived d;
int x;

d.mf1();                               // fine, calls Derived::mf1
d.mf1(x);                              // error! Base::mf1() is hidden
```

When inheritance is combined with templates, an entirely different form of the "inherited names are hidden" issue arises.

**Takeaways**
* Names in derived classes hide names in base classes. Under public inheritance, this is never desirable
* To make hidden names visible again, employ using declarations or forwarding functions

### Differentiate between inheritance of interface and inheritance of implementation

Public inheritance turns out to have two different notions, inheritance of interface (like a declaration) and inheritance of impl (like a definition).
Sometimes you want the derived to inherit only the declaration, sometimes you want both but derived can override the impl, and sometimes you want both but derived cannot override.

Consider
```cpp
class Shape {
public:
  virtual void draw() const = 0;

  virtual void error(const std::string& msg);

  int objectID() const;

  ...
};
class Rectangle: public Shape { ... };
class Ellipse: public Shape { ... };
```
Member function interfaces are always inherited.
`Shape` is an instance and cannot be instantiated.
Taking a look at each function, `draw` is pure virtual and has to be redeclared by any concrete class that inherits them, and they typically have no definition in the abstract class.
The purpose of a pure virtual is to have derived classes inherit an interface only.
From base to derived: you have to provide a `draw` impl, but I've no idea of your impl details.

Incidentally, it is possible to provide a definition for pure virtual functions, but the only way to call them would be with the class qualifiers. Like
```cpp
Shape *ps1 = new Rectangle;         // fine
ps1->Shape::draw();
```

`error` is a simple virtual function, whose purpose is to have derived classes inherit from a function interface as well as an impl. 
From base to derived: you've got to support an `error` call, but if you don't write your own, you can fall back on the default one in `Shape`.

This is potentially dangerous in that if a default impl is provided, and a client needing to override the default impl forgot to do so.

Say we have an `Airplane` base class which used to have a simple virtual `fly` function, since all children derived from it flew the same way.
Now we have another model deriving from `Airplane`, and it's intended to overwrite `fly`, but might forget to do so.
We want to maintain the behavior of having a default impl, but also want to make sure the clients are aware they may have to override `fly`.

We could make `fly` pure virtual like this, and provide a `defaultFly`:
```cpp
class Airplane {
public:
  virtual void fly(const Airport& destination) = 0;

  ...

protected:
  void defaultFly(const Airport& destination);
};

void Airplane::defaultFly(const Airport& destination) {
  default code for flying an airplane to the given destination
}
```
The derived classes that can use the default behavior do this:
```cpp
class ModelA: public Airplane {
public:
  virtual void fly(const Airport& destination)
  { defaultFly(destination); }
  ...
};

class ModelB: public Airplane {
public:
  virtual void fly(const Airport& destination)
  { defaultFly(destination); }
  ...
};
```
And for derived classes that cannot use the default behavior, they would be forced to consider if they can use `defaultFly` when implementing `fly` override.
```cpp
class ModelC: public Airplane {
public:
  virtual void fly(const Airport& destination);
  ...
};

void ModelC::fly(const Airport& destination) {
  code for flying a ModelC airplane to the given destination
}
```
This is not fool proof, but safer than just a simple virtual function.
`defaultFly` is protected as it's implementation detail of `Airplane` and its derived, and should be of no concern to the clients of `Airplane`.
`defaultFly` also should not be virtual, because no derived classes should override one such default behavior. (If it's virtual, you open yourself up to this question again, what if it's meant to be overridden by some clients but they forget to do it?)

To achieve this, you may instead do the trick of providing an impl to a pure virtual function.
```cpp
class Airplane {
public:
  virtual void fly(const Airport& destination) = 0;
  ...
};


void Airplane::fly(const Airport& destination)     // an implementation of
{                                                  // a pure virtual function
  default code for flying an airplane to
  the given destination
}

class ModelA: public Airplane {
public:
  virtual void fly(const Airport& destination)
  { Airplane::fly(destination); }
  ...
};

class ModelB: public Airplane {
public:
  virtual void fly(const Airport& destination)
  { Airplane::fly(destination); }
  ...
};

class ModelC: public Airplane {
public:
  virtual void fly(const Airport& destination);
  ...
};

void ModelC::fly(const Airport& destination) {
  code for flying a ModelC airplane to the given destination
}
```
This is almost exactly the same as above, except that `Airplane::fly` takes the place of `Airplane::defaultFly`.
In essence, this splits `Airplane::fly` into two parts, the declaration which derived has to use, and the impl which derived has the option to use only if they explicitly request it.

In merging `fly` and `defaultFly` though, you lose the ability to specify the protection level of `defaultFly`.

Now to `Shape`'s non virtual function `objectID`, which is an invariant over specialization, because it identifies behavior that is not supposed to change no matter how specialized a derived class becomes.
As such, a non virtual function specifies both the interface and impl should be inherited.

From base to derived: you must support an `objectId` that is always computed the same way.
Because this is meant to be an invariant, derived should not redefine this function, which is covered in Item 36.

The pure virtual, simple virtual, and non virtual allow you to specify whether just an interface or impl as well should be inherited.

Avoid mistakes like declaring no functions virtual, as almost any classes intended to be used as base will have virtual methods (dtor at least). (If you are worried about the performance, think about the 80-20 rule. Spend your effort optimizing where it matters.)

Also avoid the mistake of declaring all functions virtual in a base class where some invariant functions should be preserved in the inheritance chain.

**Takeaways**
* Inheritance of interface is different from inheritance of implementation. Under public inheritance, derived classes always inherit base class interfaces
* Pure virtual functions specify inheritance of interface only
* Simple (impure) virtual functions specify inheritance of interface plus inheritance of a default implementation
* Non-virtual functions specify inheritance of interface plus inheritance of a mandatory implementation

### Consider alternatives to virtual functions

Suppose that you are designing a game where different game characters have different ways of calculating health values.
```cpp
class GameCharacter {
public:
  virtual int healthValue() const;        // return character's health rating;
  ...                                     // derived classes may redefine this
};
```
The fact that `healthValue` is not pure virtual indicates there is a default way of calculating it.

While this approach might seem obvious, there are possible alternatives.

##### Non virtual interface idiom

Let's begin with the interesting school of thought that virtual functions should almost always be private.
Adherents to this school would make `healthValue` a non-virtual public method that calls into a virtual private method `doHealthValue`, like this
```cpp
class GameCharacter {
public:
  int healthValue() const               // derived classes do not redefine
  {                                     // this — see Item 36

    ...                                 // do "before" stuff — see below

    int retVal = doHealthValue();       // do the real work

    ...                                 // do "after" stuff — see below

    return retVal;
  }
  ...

private:
  virtual int doHealthValue() const     // derived classes may redefine this
  {
    ...                                 // default algorithm for calculating
  }                                     // character's health
};
```
This is called NVI idiom (non-virtual interface), for having a non-virtual public function call a private virtual method.
A particular manifestation of a more general pattern called Template Method (which has nothing to do with C++ templates).

An advantage of this comes from the "do before stuff" and "do after stuff" comments: some code can be guaranteed to be executed before and after the virtual function does the work, e.g. context setup and teardown (lock a mutex, making a log entry, making sure invariants are satisfied).

NVI means clients redefine something private, something they can't call!
But there is no design contradiction there: redefining a virtual function specifies how something is to be done. Calling a virtual function specifies when it will be done. These concerns are independent.
NVI opens up the how, but not the when.
The NVI idiom does not mandate the function to be overridden is private: protected is also common should its functionality be exposed to derived classes.

##### The strategy pattern via function pointers

A more dramatic change suggests `healthValue` need not be part of a `GameCharacter` class. Like this.
```cpp
class GameCharacter;                               // forward declaration

// function for the default health calculation algorithm
int defaultHealthCalc(const GameCharacter& gc);

class GameCharacter {
public:
  typedef int (*HealthCalcFunc)(const GameCharacter&);

  explicit GameCharacter(HealthCalcFunc hcf = defaultHealthCalc)
  : healthFunc(hcf)
  {}

  int healthValue() const
  { return healthFunc(*this); }

  ...

private:
  HealthCalcFunc healthFunc;
};
```
This can be seen as a strategy design pattern: it offers the flexibility of having different `healthValue` calculation for different instances of characters (not just different types of characters), also `healthValue` can be changed at runtime.

On the other hand, `healthValue` not being a member means it does not have access to internal parts of the object whose health it's calculating, and this becomes an issue when not all the info needed to calculated health is public.
As a general rule the only way to work around this is to weaken encapsulation: either by declaring the method a friend, or expose public getters to parts that it would otherwise have kept hidden.

##### Strategy pattern via `std::function`

`std::function` can hold any callable entity (function pointer, function object, member function pointer, etc) whose signature is compatible with what's expected.
It would look something like this
```cpp
class GameCharacter;                                 // as before
int defaultHealthCalc(const GameCharacter& gc);      // as before

class GameCharacter {
public:
   // HealthCalcFunc is any callable entity that can be called with
   // anything compatible with a GameCharacter and that returns anything
   // compatible with an int; see below for details
   typedef std::function<int (const GameCharacter&)> HealthCalcFunc;

   explicit GameCharacter(HealthCalcFunc hcf = defaultHealthCalc)
   : healthFunc(hcf)
   {}

   int healthValue() const
   { return healthFunc(*this);   }

   ...

private:
  HealthCalcFunc healthFunc;
};
```
In this case, by compatible we mean the `std::function` can contain any functions whose parameter can implicitly convert to `const GameCharacter&` and return value can implicitly convert to `int`.

The difference with the function pointer approach is mininal, except that the client now has slightly more flexibility.
```cpp
short calcHealth(const GameCharacter&);          // health calculation
                                                 // function; note
                                                 // non-int return type

struct HealthCalculator {                        // class for health
  { ... }                                        // objects
};

class GameLevel {
public:
  float health(const GameCharacter&) const;      // health calculation
  ...                                            // mem function; note
};                                               // non-int return type


class EvilBadGuy : public GameCharacter {         // as before
  ...
};

class EyeCandyCharacter : public GameCharacter {  // another character
  ...                                             // type; assume same
};                                                // constructor as
                                                  // EvilBadGuy

EvilBadGuy ebg1(calcHealth);                      // character using a
                                                  // health calculation
                                                  // function

EyeCandyCharacter ecc1(HealthCalculator());       // character using a
                                                  // health calculation
                                                  // function object

GameLevel currentLevel;
...
EvilBadGuy ebg2(                                  // character using a
  std::bind(&GameLevel::health,                   // health calculation
          currentLevel,                           // member function;
          std::placeholders::_1_1)                // see below for details
);
```
`std::function` offers a lots of flexibility in the form of what can be given.
For example, the `std::bind` allows you to specify a particular function (in this case a member function with 2 parameters, first one being an implicit `this` pointer) to call, and adapt that to the number of parameters expected by the `std::function` object.

##### The classic strategy pattern

With just strategy pattern, you could end up with something like this:
```cpp
class GameCharacter;                            // forward declaration

class HealthCalcFunc {
public:
  ...
  virtual int calc(const GameCharacter& gc) const
  { ... }
  ...
};

HealthCalcFunc defaultHealthCalc;

class GameCharacter {
public:
  explicit GameCharacter(HealthCalcFunc *phcf = &defaultHealthCalc)
  : pHealthCalc(phcf)
  {}

  int healthValue() const
  { return pHealthCalc->calc(*this);}
  ...
private:
  HealthCalcFunc *pHealthCalc;
};
```
This becomes more like a standard strategy pattern implementation, where `HealthCalcFunc` class can be derived from to customize how health calculation is done.

##### To recap

We introduced the following alternatives to public virtual methods:
* NVI idiom (private virtual being called into by public non-virtual)
* Holding a function pointer data member (per object customization allowed but restricted to working with common parts)
* Holding a `std::function` data member (similar with above but more flexibility in what can be given)
* Holding a `HealthCalcFunc` pointer data member (full fledged `Strategy` pattern where `HealthCalcFunc` can be inherited to customize per object calc behavior)

There are lots of alternatives in OO design, explore them often.

**Takeaways**
* Alternatives to virtual functions include the NVI idiom and various forms of the Strategy design pattern. The NVI idiom is itself an example of the Template Method design pattern
* A disadvantage of moving functionality from a member function to a function outside the class is that the non-member function lacks access to the class's non-public members
* `std::function` objects act like generalized function pointers. Such objects support all callable entities compatible with a given target signature

### Never redefine an inherited non-virtual function

Consider this code,
```cpp
class B {
public:
  void mf();
  ...
};
class D: public B { ... };

D x;                              // x is an object of type D

// you'd be quite surprised if the following two behave differently
B *pB = &x;                       // get pointer to x
pB->mf();                         // call mf through pointer

D *pD = &x;                       // get pointer to x
pD->mf();                         // call mf through pointer
```
How can these two differ?
They differ if `mf` is a non virtual function in `B` but redefined in `D`.

Like this
```cpp
class D: public B {
public:
  void mf();                      // hides B::mf; see Item 33
  ...
};
pB->mf();                         // calls B::mf
pD->mf();                         // calls D::mf
```
Reasoning for this is that non-virtual functions are statically bound, decided at compile time according to the type the pointer points to.

This kind of behavior is undesirable in that an object may behave as `B` or `D` which is decided by compile-time type as opposed to what the object really is.
References demonstrate similar behaviors as pointers.

What's more, item 34 described declaring a non-virtual function in base class conveys the idea that the function is invariant over specialization for that class.
Now if `D` redefines this function, there is a contradiction in the design.

This echos with item 7's declare dtors virtual in base classes: if they are not virtual, you'll hide base classes's dtor.
And should an object of the derived type be referred to using pointer to base type, only the base part will be dtor'ed afterwards.

**Takeaways**
* Never redefine an inherited non-virtual function

### Never redefine a function's inherited default parameter value

Virtual functions are dynamically bound, but default parameters are statically bound.

Reminder on dynamic type and static type:
```cpp
// a class for geometric shapes
class Shape {
public:
  enum ShapeColor { Red, Green, Blue };

  // all shapes must offer a function to draw themselves
  virtual void draw(ShapeColor color = Red) const = 0;
  ...
};

class Rectangle: public Shape {
public:
  // notice the different default parameter value — bad!
  virtual void draw(ShapeColor color = Green) const;
  ...
};

class Circle: public Shape {
public:
  virtual void draw(ShapeColor color) const;
  ...
};

Shape *ps;                       // static type  = Shape*,
                                 // dynamic type doesn't refer to anything yet
Shape *pc = new Circle;          // static type  = Shape*,
                                 // dynamic type = Circle*
Shape *pr = new Rectangle;       // static type  = Shape*,
                                 // dynamic type = Rectangle*
// dynamic type changes as assignments go
```

This is all fine, but imagine invoking a virtual function with a default parameter: the default parameter will be statically bound while the function is dynamically bound.
Now with the above example, if you call `draw` with no arguments on `pr`, it'll actually use `Color::Red`, as opposed to `Rectangle`'s default `Color::Green`.

Similarly with references.

Why does C++ statically bind default parameters for dynamically bound functions (virtual functions)? For efficiency.

Now what about keep using the same default parameters?
```cpp
class Shape {
public:
  enum ShapeColor { Red, Green, Blue };
  virtual void draw(ShapeColor color = Red) const = 0;
  ...
};

class Rectangle: public Shape {
public:
  virtual void draw(ShapeColor color = Red) const;
  ...
};
```
Code duplication and dependency: should something change in either, the other has to be updated as well.
If you find yourself doing this, consider alternatives pointed out by item 35, like NVI:
```cpp
class Shape {
public:
  enum ShapeColor { Red, Green, Blue };

  void draw(ShapeColor color = Red) const           // now non-virtual
  {
    ...                                             // set up
    doDraw(color);                                  // calls a virtual
    ...                                             // tear down
  }
  ...
private:
  virtual void doDraw(ShapeColor color) const = 0;  // the actual work is
};                                                  // done in this func

class Rectangle: public Shape {
public:
  ...
private:
  virtual void doDraw(ShapeColor color) const;       // note lack of a
  ...                                                // default param val.
};
```
This makes it clear that `draw` being an invariant (_how_ can be substituted, but not the _when_), its default parameter should always be `Red`.

**Takeaways**
* Never redefine an inherited default parameter value, because default parameter values are statically bound, while virtual functions — the only functions you should be overriding — are dynamically bound

### Model "has-a" or "is-implemented-in-terms-of" through composition

Composition is the relationship between types that arises when objects of one type contain objects of another type.

Example:
```cpp
class Address { ... };             // where someone lives

class PhoneNumber { ... };

class Person {
public:
  ...

private:
  std::string name;               // composed object
  Address address;                // ditto
  PhoneNumber voiceNumber;        // ditto
  PhoneNumber faxNumber;          // ditto
};
```
`Person` objects are composed of `string`, `Address`, and `PhoneNumber` objects.

Composition means either “has-a” or “is-implemented-in-terms-of.” 

Most people have little difficulty differentiating "has-a" and "is-a", but how about differentiating "implemented-in-terms-of" and "is-a"?

Consider you want to implement a `set` (no duplicated elements) based on `std::list`, since you don't want to pay the space cost of a binary search tree of `std::set` (three pointers)

How about having this `set` derive from `std::list`?
```cpp
template<typename T>                       // the wrong way to use list for Set
class Set: public std::list<T> { ... };
```
This may seem fine but something is quite wrong: a `set` is not a `list` as a `list` allow duplicated elements but a `set` does not.

The right way here is to suggest a `set` is implemented in terms of a `list`.
```cpp
template<class T>                   // the right way to use list for Set
class Set {
public:
  bool member(const T& item) const;

  void insert(const T& item);
  void remove(const T& item);

  std::size_t size() const;

private:
  std::list<T> rep;                 // representation for Set data
};

// and the implementation could look like
template<typename T>
bool Set<T>::member(const T& item) const {
  return std::find(rep.begin(), rep.end(), item) != rep.end();
}
template<typename T>
void Set<T>::insert(const T& item) {
  if (!member(item)) rep.push_back(item);
}
template<typename T>
void Set<T>::remove(const T& item) {
  typename std::list<T>::iterator it =               // see Item 42 for info on
    std::find(rep.begin(), rep.end(), item);         // "typename" here
  if (it != rep.end()) rep.erase(it);
}
template<typename T>
std::size_t Set<T>::size() const {
  return rep.size();
}
```
One can argue this `set`'s interface would be easier to use correctly and harder to use incorrectly if it conforms with STL container's interface, but that would require a lot more stuff to the code and better not added for the sake of clarity here.

**Takeaways**
* Composition has meanings completely different from that of public inheritance
* In the application domain, composition means has-a. In the implementation domain, it means is-implemented-in-terms-of

### Use private inheritance judiciously

Let's look at a private inheritance example
```cpp
class Person { ... };
class Student: private Person { ... };     // inheritance is now private

void eat(const Person& p);                 // anyone can eat

void study(const Student& s);              // only students study

Person p;                                  // p is a Person
Student s;                                 // s is a Student

eat(p);                                    // fine, p is a Person

eat(s);                                    // error! a Student isn't a Person
```

How does private inheritance behave?
* Compilers will generally not convert a derived class object into a base class object.
* Members inherited from a private base class become private members of the derived class, even if they were protected or public in the base class.

What does private inheritance mean?
It means is-implemented-in-terms-of. You do so because you want the derived to take advantage of some of the features available in the base, not because of there is any conceptual relationship between objects of the base and the derived.
As such, private inheritance is an implementation technique, and means nothing during software design.

Using the terms from Item 34, private inheritance means that implementation only should be inherited; interface should be ignored.

Item 38 suggests composition also can mean "is-implemented-in-terms-of", what to choose between the two?
Use composition whenever you can, and use private inheritance only when you must.

When must you?
Primarily when protected members and/or virtual functions enter the picture.

Let's say we have a `Widget` class and we now want to keep track of how many times each member function is called.
Suppose we already have this `Timer` class
```cpp
class Timer {
public:
  explicit Timer(int tickFrequency);
  virtual void onTick() const;          // automatically called for each tick
  ...
};
```
`Timer` can be configured to tick with whatever frequency we need, and on each tick, it calls a virtual function.
We can redefine the virtual function so that it examines the current state of the `Widget` world.

In order for `Widget` to redefine a virtual function in `Timer`, `Widget` must inherit from `Timer`, but public inheritance is not appropriate in this case. (not is-a, encourages misuses)

We then have
```cpp
class Widget: private Timer {
private:
  virtual void onTick() const;           // look at Widget usage data, etc.
  ...
};
```
Making `onTick` to private to not expose to client (mis)usage of this implementation detail.

This is a nice design, but achievable without private inheritance: make a Widget that publicly inherit from `Timer`, redefine `Timer` there, and put an object of that type inside `Widget`.
Like this
```cpp
class Widget {
private:
  class WidgetTimer: public Timer {
  public:
    virtual void onTick() const;
    ...
  };   // nested within Widget
  WidgetTimer timer;
  ...
};
```
This may seem more complicated, but two reasons you might want to go with this:
* You might want to allow `Widget` to be derived from, but you might want to prevent derived classes from redefining `onTick` (like `final` in Java. If `Widget` inherits from `Timer`, that's not possible, not even if the inheritance is private
* You might want to minimize `Widget`'s compilation dependencies. If `Widget` inherits from `Timer`, `Timer`'s definition must be available when `Widget` is compiled, so `Widget` header has to be included. If `WidgetTimer` is moved out of `Widget` and `Widget` contains only a pointer to `WidgetTimer`, `Widget` can get by with a simple declaration for the `WidgetTimer` class

There is other very edgy case where private inheritance might save space: when you are dealing with a class that has no data in it.
"Freestanding" empty class in C++ has non-0 size. If you do this
```cpp
class Empty {};                      // has no data, so objects should
                                     // use no memory
class HoldsAnInt {                   // should need only space for an int
private:
  int x;
  Empty e;                           // should require no memory
};
```
You'll find that `sizeof(HoldsOfInt) > sizeof(int)`.
A `char` is usually silently inserted into `Empty`, and compilers add padding to `HoldsAnInt`.

Now if we have a `Empty` as base, and `HoldsAnInt` derive from it, you are almost sure to find that `sizeof(HoldsAnInt) == sizeof(int)`.
An object of `HoldsAnInt` is not freestanding, and the base part of it needn't have a non-0 size.
This is known as empty base optimization (EBO). EBO is generally only viable under single inheritance.

In practice, empty classes aren't truly empty.
They never have non-static data members, they often contain `typedef`s, `enum`s, static data members, or non-virtual functions.
The STL has many such. `unary_function`, `binary_function` are too.

But let's go back to the basics, both private inheritance and composition mean is-implemented-in-terms-of, but composition is easier to understand, so use it whenever you can.

**Takeaways**
* Private inheritance means is-implemented-in-terms of. It's usually inferior to composition, but it makes sense when a derived class needs access to protected base class members or needs to redefine inherited virtual functions
* Unlike composition, private inheritance can enable the empty base optimization. This can be important for library developers who strive to minimize object sizes

### Use multiple inheritance judiciously


