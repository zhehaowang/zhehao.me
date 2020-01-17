### Choose carefully among erasing options

Say you have a container `Container<int> c` and you want to remove all entries with value 19 from the container, different containers have different ways to do it.

With a contiguous memory container (`string`, `vector`, `deque`), the best approach is the erase-remove idiom:
```
c.erase(remove(c.begin(), c.end(), 18), c.end());
```
This is `linear` for contiguous memory containers.

This approach also works for `list` but just `c.remove` is more efficient.

When `c` is a standard associative container, there is no member named `remove`, and using `remove` algorithm might overwrite container values.

For associative containers, the proper way is `c.erase(19)`. This is log time for `map`, and has the advantage of being based on equivalence instead of equality.

Now if instead we want to remove every element in `c` where the predicate `bool badValue(int x)` returns true.

For `string`, `vector`, `deque`, all we need to do is replacing `remove` with `remove_if`: `c.erase(remove_if(c.begin, c.end(), badValue), c.end())`

For `list`, it's `c.remove_if(badValue)`.

For associative containers (note that this may not include hash-based containers), two approaches: first one is easier to write:
```
Container<int> c; // associative
Container<int> goodValues;
remove_copy_if(c.begin(), c.end(), inserter(goodValues, goodValues.end()), badValue);
c.swap(goodValues);
```

This pays the unnecessary cost of copying, instead we can write a loop as associative containers offer no `remove_if`.
Note that calling `erase` on an associative container invalidates the iterator pointing to the element being erased, so we need to hold on to the next iterator, before we call `erase`.
```
Container<int> c;
for (auto i = c.begin(); i != c.end();) {
    if (badValue(*i)) {
        c.erase(i++);  // this works as the value of this expression is i, but
                       // as a side effect i is incremented, before erase begins
                       // executing.
    } else {
        ++i;
    }
}
// also note that since C++11 map.erase returns an iterator.
```
Note that this code won't work for `vector`, `string`, `deque` as for such containers calling `erase` not only invalidates the iterator pointing to erased elements, but also all iterators beyond the erased element.

In this case, we can instead use `erase`'s return value.
```
for (auto i = c.begin(); i != c.end(); ) {
    if (badValue(*i)) {
        i = c.erase(i);
    } else {
        ++i;
    }
}
// note that this general approach would work not just for vector, deque,
// string for code post C++11
```

For `list`, both approaches would work.

**Takeaway**
* to eliminate all objects in a container with a particular value:
  * `vector`, `string`, `deque`: erase-remove idiom
  * `list`: `list::remove`
  * associative container: `container::erase`
* to eliminate all objects in a container that satisfies a predicate:
  * `vector`, `string`, `deque`: erase-remove_if idiom
  * `list`: `list::remove_if`
  * associative containers: `remove_copy_if` + `swap`, or write a loop with post-increment on the iterator you pass to `erase`
* to do something in addition to erasing objects:
  * sequence containers: write a loop, assign the iterator with `erase`'s return value
  * associative containers: write a loop, post-increment on the iterator you pass to `erase`
