# Prefer member functions to algorithms with the same name

Sorted associative containers offer `count`, `find`, `lower_bound`, `upper_bound` and `equal_range`.
`list` offers `remove`, `remove_if`, `unique`, `sort`, `merge` and `reverse`.

Most of the time you'll want to use the member functions instead of algorithms.
The former's usually faster and integrate better with the containers (especially for associative containers).

Consider this code
```
set<int> s;
...
set<int>::iterator i  = s.find(727);                      // logarithmic
set<int>::iterator i1 = find(s.begin(), s.end(), 727);    // linear
```

Most STL implementations use red-black trees for associative containers, which may be out of balance by up to a factor of two.
Although the maximum number of comparisons is worse than that of perfectly balanced binary trees, in practice the overall performance of perfectly balanced binary trees are often worse than red-black trees.

Efficiency is not the only reason.
In this example, `find` algorithm searches for 727 using equality, `find` member uses equivalence, which is what the associative container uses.
Similar difference holds for member vs algorithm versions of `find`, `count`, `lower_bound`, etc.

This difference is especially pronounced when working with `map`s and `multimaps`, as they hold `pair` objects, and their member functions only checks for equivalence on the key part of the each pair.
The algorithm versions checks equality on the entire pair.

In the case of `list`, the story is almost completely about efficiency: each of the algorithms that list specializes copie objects, but list-specific versions copy nothing: they simply manipulate the pointers connecting list nodes.
The asymptotic complexity is the same in this case, but you'd save linear number of element copies.

Also note that `list` member often behaves differently from algorithm counterparts: algorithm `remove`, `remove_if`, `unique` must be followed by `list.erase` if you really want to erase, but `list`'s versions of them require no further `erase`.

Finally algorithm `sort` cannot be applied on `list` as it only has bidirectional iterators, and algorithm `merge` behaves differently from `list.merge` as the algorithm version is not permitted to modify source ranges but `list.merge` always modifies the lists it works on.
