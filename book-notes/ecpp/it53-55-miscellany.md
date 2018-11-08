# Miscellany

### Pay attention to compiler warnings

In C++, it's a good bet the compiler writer konws better about what's going on than you do.

Consider this code
```cpp
class B {
public:
virtual void f() const;
};

class D: public B {
public:
virtual void f();
};
```

While the intention is to redefine `B::f` in `D::f`, what actually happens is `D::f` hides `B::f`.
Some compilers would emit a warning for this.

It's generally best practice to write code that compiles warning free at the highest warning level.

Warnings are compiler implementation dependent, so don't get sloppy in your coding and hope that compilers spot the error for you.

**Takeaways**
* Take compiler warnings seriously, and strive to compile warning-free at the maximum warning level supported by your compilers
* Don't become dependent on compiler warnings, because different compilers warn about different things. Porting to a new compiler may eliminate warning messages you've come to rely on

### Familiarize yourself with the standard library, including tr1

The standard for this book was ratified in 1998, and 2003 saw a minor bug-fix update.
C++0x was meant to be a follow-up to 2003.
tr1 heralds the new features in a new release of C++.

Major parts of standard library as specified by C++98:
* STL, containers, iterators, algorithms, function objects and their adapters
* Iostreams
* Support for internationalization (unicode chars, `wstring`, `wchar_t`)
* Support for numeric processing, complex numbers, arrays of pure values (`valarray`)
* An exception hierarchy, `exception`, from which `logic_error`, `runtime_error`, etc derive
* C89's standard library

TR1 of C++98 has:
* `shared_ptr`, `weak_ptr`
* `function`, to represent any callable entity, e.g. `function<std::string (int)>`
* `bind`
* Hash tables, `unordered_map`, `unordered_multimap`, `unordered_set`, `unordered_multiset`
* Regular expressions
* `tuple`
* `array`, fixed size during compilation, no dynamic memory
* `mem_fun`, adapting member function pointers
* `reference_wrapper`, as if containers hold references
* Random number generation
* Mathematical special functions
* C99 compatibility extensions
And to better support templates, tmp
* Type traits, given `T`, this can reveal if `T` is a built-in type, offers a virtual dtor, is an empty class, is implicitly convertible so some `U`, proper alignment, etc
* `result_of`: deduce the return type of function calls
As pure additions, not to replace anything in standard.

Boost offers these functionality and more, sometimes not the same as specified by the standards.

**Takeaways**
* The primary standard C++ library functionality consists of the STL, iostreams, and locales. The C99 standard library is also included
* TR1 adds support for smart pointers (e.g., `shared_ptr`), generalized function pointers (`function`), hash-based containers, regular expressions, and 10 other components
* TR1 itself is only a specification. To take advantage of TR1, you need an implementation. One source for implementations of TR1 components is Boost

### Familiarize yourself with Boost

Searching for a collection of high-quality, open source, platform- and compiler-independent libraries? Look to Boost.
Interested in joining a community of ambitious, talented C++ developers working on state-of-the-art library design and implementation? Look to Boost.
Want a glimpse of what C++ might look like in the future? Look to Boost.

Boost as of the time of this book, contains libraries of these categories:
* String and text processing, type-safe `printf`, regular expressions, tokenizing and parsing
* Containers, fixed-sized arrays, variable-sized bitsets, multidimensional arrays
* Function objects and higher order programming, like lambda
* Generic programming, with extensive traits
* TMP, MPL library
* Math and numerics, rational numbers; octonions and quaternions; greatest common divisor and least common multiple computations; and random numbers
* Correctness and testing
* Data structures, like type-safe unions
* Inter-language support, like C++ with python
* Memory, like pool
* Miscellaneous, CRC checking, traversing file system, datetime, etc

Boost does not cover GUI, DB communication, etc

**Takeaways**
* Boost is a community and web site for the development of free, open source, peer-reviewed C++ libraries. Boost plays an influential role in C++ standardization
* Boost offers implementations of many TR1 components, but it also offers many other libraries, too
