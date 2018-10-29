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

