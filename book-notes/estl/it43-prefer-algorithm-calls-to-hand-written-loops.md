# Programming with the STL

# Prefer algorithm calls to hand-written loops

Internally, algorithms are loops.
Many things doable with a loop can be rewritten to use algorithms. E.g.
```
class Widget {
  public:
    ...
    void redraw() const;
    ...
};

list<Widget> lw;
...
for (list<Widget>::iterator i = lw.begin(); i != lw.end(); ++i) {
    i->redraw();
}

// alternatively
for_each(lw.begin(), lw.end(), mem_fun_ref(&Widget::redraw));
```

Many prefer reading and writing loops but here are a few reasons why algorithm might be preferable:

* efficiency

Looking at the above example, one minor point is how many times `list.end` has to be evaluated: our loop does not modify the `list` so ideally `list.end` needs to be called only once.
This is not the case with the loop, but is the case with our algorithm call.
To be fair, `list.end` is almost certainly inline so most compilers will be able to avoid repeated computations by hoisting their results out of the loops.

The first major argument for efficiency is that library implementers can take advantage of their knowledge of container implementations to optimize traversals.
E.g. a `deque` typically store objects in one or more fixed size arrays. Pointer-based traversals of these arrays are faster than iterator-based traversals, but only library implementers can use pointer-based traversals because they know the size of the internal arrays and how to move from one array to the next.
Some STL algorithm implementation take their deque's internal data structure into account, making traversal even faster.
By writing raw loops and not using their call, you forego such advantage.

Second major argument is algorithms typically use algorithms that are asymptotically superior.
`sort`, search algorithms for sorted ranges, eliminating objects from contiguous memory containers, etc.

* correctness

Imagine you are given a C-style array and want to create a `deque` from it where each element in `deque` is that element in array `+41`.

You might want to do
```
// given double data[numDoubles]

deque<double> d;
for (size_t i = 0; i < numDoubles; ++i) {
    d.insert(d.begin(), data[i] + 41);
}
// this ends up with a deque in reverse order!

deque<double>::iterator insertLocation = d.begin();
for (size_t i = 0; i < numDoubles; ++i) {
    d.insert(insertLocation++, data[i] + 41);
}
// this yields undefined behavior because deque insert can invalidate iterators,
// including insertLocation

deque<double>::iterator insertLocation = d.begin();
for (size_t i = 0; i < numDoubles; ++i) {
    insertLocation = d.insert(insertLocation, data[i] + 41);
    ++insertLocation;
}
// this works, but it took us a while to get here!

transform(
    data, data + numDoubles, inserter(d, d.begin()), bind2nd(plus<int>(), 41));
// for inserter, see it30
// bind2nd may take a little to get used to, but otherwise this should be easier
// to get right than iterator invalidations
```

* maintainability

We argue algorithm is clearer, in that their edge is in a known vocabulary.
It's reasonable to expect a professional C++ programmer to know what these algorithms do, or be able to look them up.
Someone else can see your `partition`, `transform` and `replace_if` and know what they do quickly, as opposed to having to read up your raw loop and figure out.

Simply put, algorithm names suggest what they do, not so with `for` or `while`.

Same argument as not building your own wheels.
Why not? someone else has already done it, the names are standard and everyone else knows, and they may know wheel building better than you do.

In practice though, specifying what to do during an iteration can be clearer using a loop.

Suppose you want to find the first element in a `vector` whose value is greater than some `x` and smaller than some `y`.
```
// loop
vector<int> v;
int x, y;
vector<int>::iterator i = v.begin();
for (; i != v.end(); ++i) {
    if (*i > x && *i < y) {
        break;
    }
}

// algorithm
template <typename T>
class BetweenValues : public unary_function<T, bool> {
  public:
    BetweenValues(const T& _low, const T& _high) : low(_low), high(_high) {}
    bool operator()(const T& val) const {
        return val > low && val < high;
    }
  private:
    T low;
    T high;
};

vector<int>::iterator i = find_if(v.begin(), v.end(), BetweenValues<int>(x, y));
```
The algorithm version is now a lot more to write, and one has to read `BetweenValues` definition elsewhere to understand what this `find_if` is doing.
(templates can't be declared inside functions, and if instead you make `BetweenValues` merely a class to be declared in a class, it still won't work as a local class cannot bind to template type arguments such as the type of the functor taken by `find_if`).

Thus the bottom line on code clarity is that it all depends on what you need to do inside the loop.
If you need something an algorithm already does, use an algorithm, if not and you have to jump through hoops with binds, adapters, functor classes, then a raw loop probably is better.
If the loop is super long and complicated, you may want to put that in a function which shifts the favor back to algorithms with a functor implementing that function.

In general, every time we replace low-level words like `for`, `while` and do with higher-level terms like `insert`, `find` and `for_each`, we raise the level of abstraction in our software and thereby make it easier to write, document, enhance and maintain.
