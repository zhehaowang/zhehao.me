# Prefer range member functions to their single element counterparts

Given two vectors, what's the easiest way to make `v1`'s content the same as the second half of `v2`'s (assume `v2` has even number of elements)?

Your answer should not contain any loops, nor more than one call.

The easiest way is `v1.assign(v2.begin() + v2.size / 2, v2.end())`.
`assign` is a convenient beast that many programmers overlook. It's available for all standard sequence containers. Whenever you have to completely replace the contents of a container, you should think `assign`. If you are just assigning one value, use `operator=`.
In this case `assign` is also a range member function which can take two iterators. Without it you'd have to write an explicit loop, e.g.
```
vector<Widget> v1, v2;
// set both up
v1.clear();
for (vector<Widget>::const_iterator ci = v2.begin() + v2.size() / 2;
     ci != v2.end(); ++ci) {
    v1.push_back(*ci);
}
```

This is more work and less efficient than the `assign` approach. (Item 43 further explains why you should avoid explicit loops.)

One way to avoid the loop is
```
v1.clear();
copy(v2.begin() + v2.size() / 2, v2.end(), back_inserter(v1));
```
No explicit loop is present but one resides inside `copy`. As a result the efficiency penalty remains.

Almost all uses of copy where the destination range is specified usign an insert iterator (e.g. `back_inserter`, `front_iterator`) can be and should be replaced with calls to range member functions.
E.g. here we can instead do
```
v1.clear();
v1.insert(v1.end(), v2.begin() + v2.size() / 2, v2.end());
```
This is more clear in that it puts the emphasis in what's happening: data is being inserted into `v1`.

Two reasons to prefer range member functions to their single-element counterparts:
* it's generally less work to write
* range member functions tend to lead to code that is clearer and more straightforward

This is not just style.
For standard sequence containers application of single element member functions makes more demands on memory allocators, copies objects more frequently, and/or performs redundant operations compared to range member functions that achieve the same end.
Suppose you want to copy an array of ints into the front of a vector.
```
int data[numValues];
// fill it up
vector<int> v;
...
v.insert(v.begin(), data, data + numValues);
```

Doing it with an explicit loop looks like this
```
// same
vector<int>::iterator insertLoc(v.begin());
for (int i = 0; i < numValues; ++i) {
    insertLoc = v.insert(insertLoc, data[i]);
}
```
Note that we have to save `insertLoc` after each insertion, otherwise all iterations after the first would yield undefined behavior, because each `insert` invalidates `insertLoc`. (Even if not, the order would be reversed from desired.)

We could instead do
```
copy(data, data + numValues, inserter(v, v.begin()));
```
By the time copy is instantiated, the code based on copy and the above using explicit loop will be almost identical.
For purposes of efficiency analysis, we'll focus on the explicit loop.
It's less efficient than the range member function version in three ways:
* Unnecessary number of function calls. You'll call insert `insert` `numValues` times. As opposed to 1 call in range insertion. (inlining might save you this but why not go with the version that definitely doesn't have it)
* Cost of inefficiently moving the existing elements in `v` to their final post-insertion positions. In the loop version for each call we shift all subsequent elements once, and each existing element will be moved a total of `numValues` times. In contrast, the standard requires that range insertion move existing container elements into their final positions, i.e. at the cost of one move per element. (note that a range insertion function can move an element into its final position in a single move only if it can determine the distance between the two iterators without losing its place. All forward iterators offer this functionality, forward iterators are nearly ubiquitous, and all iterators for the standard containers offer forward iterator functionality; so do hash-based container iterators, and pointers acting as iterators into arrays. This does not hold for input / output iterators. so if range insertion is used on input iterators, like `istream_iterator`, then this advantage ceases to exist.)
* The single element insertion version also can cause more memory allocation: vector may double multiple times.

Similar arguments hold for string. For deque repeated memory allocation does not apply, but large number of moves generally applies.

For lists function call overhead still applies. Extra moves and extra memory allocations do not apply. A different issue arise: with single-element insertions, all but last element will set its next pointer twice. With range insertion only one element needs to set its next pointer (when connecting to the start of `v1`).

Because the range member function knows how many nodes will ultimately be inserted, it can avoid the superfluous operations mentioned above.

For sequence containers using range member function is much more than just style, for associative containers the efficiency case is harder to make (only extra function call argument holds), but they are certainly no less efficient than single element insertions.

Knowing which member functions support ranges makes it easier to recognize opportunities to use them.
* range construction. all standard containers offer this. `container::container(InputIterator begin, InputIterator end)`
* range insertion. all standard containers offer this. `container::insert(iterator position, InputIterator begin, InputIterator end)`
