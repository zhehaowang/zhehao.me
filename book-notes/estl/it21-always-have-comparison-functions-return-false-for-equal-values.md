# Always have comparison functions return false for equal values

Imagine we have this
```
set<int, less_equal<int> > s;
s.insert(10);
s.insert(10);
// we'll end up with 2 copies of 10
```
When inserting the second 10, it needs to check for equivalence with current elements (it19).
Given our comparison function is `<=`, we check `is_equivalent = !(10 <= 10) && !(10 <= 10)`, which evaluates to false meaning 10 and 10 are not equivalent.
Equal values are, by definition, not equivalent.

Given this you'll want your comparison function for associative containers always return false for equal values.
You'll need to be vigilant as this constraint is easy to violate.
E.g. this examples tries to sort string pointers by dereferenced values in descending order.
```
struct StringPtrGreater : public binary_function<const string*, const string*, bool> {
    bool operator()(const string* ps1, const string* ps2) const {
        return !(*ps1 < *ps2); // just negate the old test, this is incorrect.
    }
}

// you'll want this instead
// return *ps2 < *ps1;
```

The return value of a comparison functor `operator` indicates whether one value precedes another in the sort order defined by that function. Equal values that never precede one another, so comparison functions should always return false for equal values.

Now what about a `multiset` or a `multimap`? The comparison still should not return true on equal.
```
multiset<int, less_equal<int> > s;
s.insert(10);
s.insert(10);
```
We want the container to store both copies, but now if we do `equal_range` we expect to get back a pair of iterators that contains both copies, but as `equal_range` finds a range of equivalent not equal values, in this example 10 and 10 are not equivalent, so there's no way both can be in the range identified by `equal_range`.

Technically speaking, comparison functions used to sort associative containers must define a strict weak ordering over the objects they compare.
(Comparison functions passed to algorithms like `sort` are similarly constrained.)
