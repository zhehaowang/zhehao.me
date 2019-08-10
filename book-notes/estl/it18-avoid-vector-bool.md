# Avoid `vector<bool>`

A `vector<bool>` is not an STL container. And it doesn't hold bools.

An object becomes an STL container only if it satisfies the requiremets by the standard.
Among the requirement there is the following should compile:
```
T *p = &c[0];  // where c is container of type T and c supports operator[]
```

But with `vector<bool>`, this won't compile.
```
vector<bool> v;
bool *pb = &v[0]; // initialize a bool* with the address of what
                  // vector<bool>::operator[] returns
```

This won't compile because `vector<bool>` does not contain bools, but a packed representation of bools that is designed to save space.
Internally, `vector<bool>` uses the moral equivalent of bitfields to represent the bools it pretends to be holding.

You can represent the same information but you can't create a pointer to individual bits.

References to individual bits are forbidden, too. This creates a problem for `vector<bool>::operator[]` because it needs to somehow return a reference to a bit and there is not such thing.
To deal with this, `vector<bool>::operator[]` returns an object that acts like a reference to a bit, a so-called proxy object.

Stripped down to the bare essentials, `vector<bool>` looks like this:
```
template <typename Allocator>
vector<bool, Allocator> {
  public:
    class reference { ... }; // class to generate proxies for references to
                             // individual bits

    reference operator[](size_type n);  // operator[] returns a proxy
};

vector<bool> v;
bool *pb = &v[0];   // error! the expression on the right is of type
                    // vector<bool>::reference*, not bool*
```

What should you use when you need a `vector<bool>`?
There are two alternatives that suffice almost all the time. The first is a `deque<bool>`.
* A `deque` offers everything `vector` has except `reserve` and `capacity`, and a `deque<bool>` is an STL container that really contains bools.
* Another alternative is `bitset`. `bitset` isn't an STL container but is part of the standard library. Its size is fixed during compilation, so there is no support for inserting or erasing elements, and it offers no support for iterators. Like a `vector<bool>` it uses a compact representation that devotes only a single bit to each value it contains, it offers `vector<bool>`'s special flip member function, and a number of other functions that make sense for collection of bits. It'll serve you well if you don't need iterators or dynamic changes in size.

Why `vector<bool>` is in the standard, given that it's not a container?
The standard decided to develop `vector<bool>` as a demonstration of how the STL could support containers whose elements are accessed via proxies.
This example will make a ready reference for implementing practitioner's own proxy-based containers.
What they discovered was that it was not possible to create proxy-based containers that satisfy all the requirements of STL containers, and `vector<bool>` remained.
