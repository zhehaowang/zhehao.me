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
