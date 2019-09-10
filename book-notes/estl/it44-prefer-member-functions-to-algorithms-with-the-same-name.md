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

This difference is especially pronounced when working with `map`s and `multimaps`.