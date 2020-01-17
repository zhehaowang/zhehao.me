### Item 34: note which algorithms expect sorted range

Not all algorithms are applicable to all ranges.
E.g. `remove` requires forward iterators and the ability to make assignments through those iterators, therefore not applicable to iterators of `map`, `multimap`.
If you violate such your code won't compile and will likely have a lengthy error message.

Another type of error is more subtle: some algorithms require sorted ranges, violating these won't lead to compile error but undefined runtime behavior.

A few algorithms can work with sorted or unsorted ranges, but they are most useful with operating on sorted ranges.

Here's a list that require the range to be sorted:
```
binary_search lower_bound upper_bound equal_range
```
These use binary search and promise logarithmic time lookups if given random access iterators.
When given bidirectional iterators they still perform only a logarithmic number of comparisons, but they move to places in the range in linear time.

```
set_union set_intersection set_difference set_symmetric_difference
```
These offer linear time set operations.

```
merge inplace_merge
```
These offer linear time single pass merge as in mergesort.

```
includes
```
This decides if every object in one given range are in another.
This also promises linear time.

In general, algorithms requiring a sorted range can guarantee better complexity than working with unsorted ranges.

These typically work with sorted ranges but don't require them:
```
unique unique_copy
```
`unique` eliminates all but the first element from every consecutive group of equal elements, like `uniq` in Linux.
`unique` behaves `remove`-like in assigning subsequent elements to fill removed elements.

With many sorting possible, if you pass a sorted range to an algorithm that also takes a comparison function, be sure that the comparison function you pass behaves the same as the one you used to sort the range.
```
// don't do this
vector<int> v;
...
sort(v.begin(), v.end(), greater<int>()); // sort in descending order
...
bool exists = binary_search(v.begin(), v.end(), 5); // this assumes sorting in
                                                    // ascending order!
// do this instead
bool exists = binary_search(v.begin(), v.end(), 5, greater<int>()):
```

All the algorithms that require sorted range (the ones above except `unique` and `unique_copy`) determine the two values are the same using equivalence like `map` does.

`unique` and `unique_copy` determine this using equality, which you can override with an equality that actually does equivalence. For equality vs equivalence check out it19.

**Takeaways**
* algorithms typically expect sorted ranges so that they can do their work with a better complexity. Remember to give them only sorted ranges and a comparison function that is consistent with the one used to do the sorting.
* binary search based algorithms (including `equal_range`, `upper_bound`), set operations (`set_union`), merge and includes require sorted range. These use equivalence in deciding if two are the same.
* `unique` and `unique_copy` don't require sorted ranges but often work with they behave like `uniq`. These use equality in deciding if two are the same.
