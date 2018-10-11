# Constructors, destructors, and assignment operators

### Know what functions C++ silents writes and calls

If you don't declare a copy constructor, copy assignment operator, or destructor, C++ declares and defines one for you.
If you don't declare any constructors, C++ declares a default constructor for you.
Note that the generated dtor is not virtual, unless the class is derived from a class with a virtual dtor.

Consider this example
```cpp
template<typename T>
class NamedObject {
public:
  NamedObject(const char *name, const T& value);
  NamedObject(const std::string& name, const T& value);

  ...

private:
  std::string nameValue;
  T objectValue;
};

NamedObject<int> no1("Smallest Prime Number", 2);

NamedObject<int> no2(no1); // calls compiler-supplied copy constructor
                           // this calls copycon of string to copy
                           // nameValue, and copies the bits of int
                           // (instantiated type, in this case a primitive
                           // type) to copy objectValue.
no2 = no1;                 // copy assignment, calls the copy assignment of
                           // string, and copies over the bits of int

// in fact, compiler generated copy assignment behaves as described above
// only when resulting code is both legal and has a reasonable chance of
// making sense.

// suppose we have this instead

template<class T>
class NamedObject {
public:
  // this ctor no longer takes a const name, because nameValue
  // is now a reference-to-non-const string. The char* constructor
  // is gone, because we must have a string to refer to.
  NamedObject(std::string& name, const T& value);

  ...                               // as above, assume no
                                    // operator= is declared
private:
  std::string& nameValue;           // this is now a reference
  const T objectValue;              // this is now const
};

// then consider this code
std::string newDog("Persephone");
std::string oldDog("Satch");

NamedObject<int> p(newDog, 2);

NamedObject<int> s(oldDog, 36);

p = s;                                       // what should happen to
                                             // the data members in p?
// Should the reference in p itself be modified, to refer to s instead?
// C++ doesn't allow that.
// Should the referred-to string be modified?
// In this case, C++ refuses to compile the code.
// Similarly, if the class contains const members, copy assignment opr
// won't be generated. Or if the class derives from a base class whose
// copy assignment is made private, the class's copy assignment won't
// work. 
```

**Takeaways**
* Compilers may implicitly generate a class's default constructor, copy constructor, copy assignment operator, and destructor

### Explicitly disallow the use of compiler generated functions you don't want

As of C++03, the key to the solution is that all the compiler generated functions are public.
To prevent these functions from being generated, you must declare them yourself, but there is nothing that requires that you declare them public. Instead, declare the copy constructor and the copy assignment operator private.
By declaring a member function explicitly, you prevent compilers from generating their own version, and by making the function private, you keep people from calling it.
And you don't implement it, so that member functions or friends cannot call these private functions.
(Or, you could declare a base class Uncopyable whose copycon and assignment opr are made private, and make your class private inherit from Uncopyable, in which case, when trying to call your class's copycon or assignment opr within a friend, you'd get compiler error instead of linker error, which is preferable.)

This would be obsolete by EMC++ item 11.

**Takeaways**
* To disallow functionality automatically provided by compilers, declare the corresponding member functions private and give no implementations. Using a base class like Uncopyable is one way to do this.

### Declare dtors virtual in polymorphic base classes

Declare a virtual destructor in a class if and only if that class contains at least one virtual function. (if it does not, it usually indicates the class is not meant to be inherited from)

The bottom line is that gratuitously declaring all destructors virtual is just as wrong as never declaring them virtual.

It is possible to get bitten by the non-virtual destructor problem even in the complete absence of virtual functions. For example, the standard string type contains no virtual functions, but misguided programmers sometimes use it as a base class anyway:
```cpp
class SpecialString: public std::string {   // bad idea! std::string has a
  ...                                       // non-virtual destructor
};

std::string* ptr = new SpecialString(...);
...
// undefined behavior
delete ptr;
```
The same analysis applies to any class lacking a virtual destructor, including all the STL container types (e.g., vector, list, set, std::unordered\_map.)
Don't inherit from STL container types!

Occasionally it can be convenient to give a class a pure virtual destructor: if you want an abstract class (one that cannot be instantiated) but you don't have any pure virtual functions for it. You could give it a pure virtual dtor, but remember you have to provide a definition for this pure virtual dtor.

```cpp
class AWOV {                            // AWOV = "Abstract w/o Virtuals"
public:
  virtual ~AWOV() = 0;                  // declare pure virtual destructor
};

AWOV::~AWOV() {}                     // definition of pure virtual dtor
```
Reason for needing a definition being when destroying an object of a derived class, the base class's dtor will be called after that of the derived class, and you provide a definition to base class's dtor since otherwise the linker would complain. 

Some classes are designed to be used as base classes, yet are not designed to be used polymorphically.
Such classes — examples include Uncopyable from Item 6, are not designed to allow the manipulation of derived class objects via base class interfaces.
As a result, they don't need virtual destructors.

**Takeaways**
* Polymorphic base classes should declare virtual destructors. If a class has any virtual functions, it should have a virtual destructor
* Classes not designed to be base classes or not designed to be used polymorphically should not declare virtual destructors

### Prevent exceptions from leaving dtors

Consider this code.
```cpp
class Widget {
public:
  ...
  ~Widget() { ... }            // assume this might emit an exception
};

void doSomething()
{
  std::vector<Widget> v;
  ...
}                                // v is automatically destroyed here
```
If an exception is thrown when the first element in v is destroyed, C++ has to finish destroying all the remaining Widgets in v, say another exception is thrown then, we'll have two exceptions at the same time, which is not allowed in C++ and causes undefined behavior or program termination.

What if your dtor needs to perform an action that might fail and cause an exception to be thrown?
Suppose you have this class for DBConnection
```cpp
class DBConnection {
public:
  ...

  static DBConnection create();        // function to return
                                       // DBConnection objects; params
                                       // omitted for simplicity

  void close();                        // close connection; throw an
                                       // exception if closing fails
  
  // and you want to have an RAII class that closes connection on dtor
  ~DBConn()                            // make sure database connections
  {                                    // are always closed
    db.close();
  }
};
```
There are two primary ways to solve this issue:
One is to terminate the program if close throws, by calling abort
```cpp
DBConn::~DBConn() {
  try {
    db.close();
  } catch (...) {
    // make log entry that the call to close failed;
    std::abort();
  }
}
```
This is reasonable if the program cannot continue to run after an exception is encountered during destruction.
Calling abort forestalls undefined behavior.

The other is to swallow it.
```cpp
DBConn::~DBConn() {
  try {
    db.close();
  } catch (...) {
    // make log entry that the call to close failed;
  }
}
```
In general, suppressing exception is a bad idea, since it suppresses the important information that something failed.
However, in this case this would still be preferable to undefined behavior or premature termination.
For this to be viable, the program must be able to continue execution even if an error occurred and has been ignored.

Neither way is ideal.
In this case we could expose close to clients so that they have a chance of handling exceptions from close operation.
And if the client doesn't do it, we still fall back to aborting or swallowing.
```cpp
class DBConn {
public:
  ...

  void close()                                     // new function for
  {                                                // client use
    db.close();
    closed = true;
  }

  ~DBConn() {
    if (!closed) {
      try {                                            // close the connection
        db.close();                                    // if the client didn't
      } catch (...) {                                    // if closing fails,
        make log entry that call to close failed;      // note that and
        ...                                            // terminate or swallow
      }
    }
  }

private:
  DBConnection db;
  bool closed;
};
```

Does this violate the principle of making interfaces easy to use?
We would argue not as in this example, telling clients to call close themselves gives them an opportunity to deal with errors they would otherwise have no chance to react to.
And if they don't want to deal with it, they'd still fall back on the dtor's default action.

**Takeaways**
* Destructors should never emit exceptions. If functions called in a destructor may throw, the destructor should catch any exceptions, then swallow them or terminate the program
* If class clients need to be able to react to exceptions thrown during an operation, the class should provide a regular (i.e., non-destructor) function that performs the operation

### Never call virtual functions during ctor or dtor

Suppose you have this code.
```cpp
class Transaction {                               // base class for all
public:                                           // transactions
  Transaction();

  virtual void logTransaction() const = 0;       // make type-dependent
                                                 // log entry
  ...
};

Transaction::Transaction()                        // implementation of
{                                                 // base class ctor
  ...
  logTransaction();                               // as final action, log this
}                                                 // transaction

class BuyTransaction: public Transaction {        // derived class
public:
  virtual void logTransaction() const;          // how to log trans-
                                                // actions of this type
  ...
};
class SellTransaction: public Transaction {      // derived class
public:
virtual void logTransaction() const;            // how to log trans-
                                                // actions of this type
  ...
};

// consider what happens when this is executed:
BuyTransaction b;
```
Base class ctor would have to be called first:
The version of logTransaction that's called is the one in Transaction, not the one in BuyTransaction — even though the type of object being created is BuyTransaction.

During base class construction, virtual functions never go down into derived classes. Instead, the object behaves as if it were of the base type.
The reason for such is that while base class ctor is being called, derived class members aren't initialized yet. If we do call into derived class's overriden functions then we'd run into undefined behavior when those functions refer to members of the derived class.

Actually, during base class construction of a derived class object, the type of object is that of the base class.
Not only do virtual functions resolve to those of the base class, but also parts of the language using runtime type information (e.g., dynamic_cast and typeid) treat the object as a base class type: during base class ctor, the derived part does not exist yet, so it's best to treat the object's type as that of the base.
An object doesn't become a derived class object until execution of a derived class constructor begins.

The same reasoning applies during destruction.
Once a derived class destructor has run, the object's derived class data members assume undefined values, so C++ treats them as if they no longer exist.
Upon entry to the base class destructor, the object becomes a base class object, and all parts of C++ — virtual functions, dynamic_casts, etc., — treat it that way.

In the above example code's case, it shouldn't link as logTransaction in base class is pure virtual.
Some compilers would also issue warnings about this.

This more insidious version, however, will likely compile and link:
```cpp
class Transaction {
public:
  Transaction()
  { init(); }                                      // call to non-virtual...

  virtual void logTransaction() const = 0;
  ...

private:
  void init()
  {
    ...
    logTransaction();                              // ...that calls a virtual!
  }
};
```
The only way to avoid this problem is to make sure that none of your constructors or destructors call virtual functions on the object being created or destroyed and that all the functions they call obey the same constraint.

How do we achieve what we wanted to do then?
One way is to not make logTransaction virtual, but rather parameterize the information it needs.
Like this
```cpp
class Transaction {
public:
  explicit Transaction(const std::string& logInfo);

  void logTransaction(const std::string& logInfo) const;   // now a non-
                                                           // virtual func
  ...
};

Transaction::Transaction(const std::string& logInfo)
{
  ...
  logTransaction(logInfo);                                // now a non-
}                                                         // virtual call

class BuyTransaction: public Transaction {
public:
BuyTransaction( parameters )
: Transaction(createLogString(parameters ))             // pass log info
  { ... }                                               // to base class
   ...                                                  // constructor

private:
  static std::string createLogString( parameters );
};
```
Note the private static function createLogString, using a helper function like this is often more readable, and making it avoids accidentally using the data members of BuyTransaction in createLogString, whose uninitialized state is the reason why we parameterize the message in the first place.

**Takeaways**
* Don't call virtual functions during construction or destruction, because such calls will never go to a more derived class than that of the currently executing constructor or destructor

### Have assignment operators return a reference to \*this

