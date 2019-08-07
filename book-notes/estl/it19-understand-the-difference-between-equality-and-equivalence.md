# (Sorted) Associative containers

Sorted. View their content through the lens of equivalence instead of equality.

# Understand the difference between equality and equivalence

`find` algorithm and `set::insert` are representative of many methods that decide if two values are the same.
`find`'s definition of the same is equality based on `operator==`.
`set::insert` is based on equivalence, which is usually based on operators.
It's possible the two differ.

Equality means if `x == y` returns true, then they have equal values, otherwise they don't.
Not all members of `x` and `y` have to be the same.

Equivalence is based on the relative ordering of object values in a sorted range.
Two objects `x` and `y` have equivalent values with respect to the sort order used by an associative container `c` if neither precedes the other in `c`'s sort order.

E.g. consider a `set<Widget>`, `w1` and `w2` have equivalent values with respect to `s` if neither precedes the other in `s`'s sort order (`less<Widget>`, who by default simply calls `Widget::operator<`)
So `x` and `y` have equivalent values with respect to operator if the following expression is true: `!(w1 < w2) && !(w2 < w1)`.

In the general case, the comparison function for an associative container isn't `operator<` or `less`, but a user-defined predicate.
Every standard associative container makes its sorting predicate available through its `key_comp` member function.
Two objects `x` and `y` have equivalent values with respect to an associative container `c`'s sorting criterion if the following evaluates to true:
```
!c.key_comp()(x, y) && !c.key_comp()(y, x)
```

Imagine you have a case insensitive `set<string, CaseInsensitiveComparator> s`, `s.insert("P")` and `s.insert("p")` will cause only the first to be inserted.
`if (s.find("p") != s.end())` will evaluate to true, and `if (find(s.begin(), s.end(), "p") != s.end())` will evaluate to false.
This is because `"p"` is equivalent to `"P"` with respect to `CaseInsensitiveComparator` but is not equal to it.
This demonstrates one reason you should prefer member functions like `set::find` to their non-member counterparts (like `std::find`).

Why do standard associative containers use equivalence instead of equality?
As they are kept in sorted order, each container must have a comparison function (`less` by default) that defines how to keep things sorted.
If we use equality instead of equivalence, each container would need a second comparison function for determining equal.
This can create a problem: say we have `set<string, CaseInsensitiveLess, equal_to<string>>`, then when we `insert` `"p"` and `"P"` `equal_to` will decide both can be inserted as they are unequal, but `CaseInsensitiveLess` cannot decide which goes first. This could introduce undeterminism (btw, standard also does not place constraints on equivalent values ordering in `multiset` and `multimap`)

The long and short of it is that by using only a single comparison function and by using equivalence as the arbiter of what means for two values to be the same, the standard associative containers sidestep a whole bunch of issues if two comparison functions were allowed.

Once you leave the realm of sorted associative containers, the situation changes.
Custom hash-based containers can base on equivalence or equality.

**Takeaways**
* equality means `operator==`, `std::find` uses this by default.
* equivalence (with respect to a container's `less` / `key_comp`) means `!less(x, y) && !less(y, x)`, `set::insert`, `set::find` use this by default.
* the two can disagree, and are important to differentiate when dealing with standard sorted associative containers.
