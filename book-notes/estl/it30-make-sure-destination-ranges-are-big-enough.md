### Algorithms

### Make sure destination ranges are big enough

Containers expand automatically when we go over its capacity, but not algorithms!

Consider this
```
int transform_int(int x);  // produces some new value from x

vector<int> values;
... // fill values with ints

vector<int> results;
transform(values.begin(), values.end(), results.end(), transform_int);
// apply transform_int on each element of values and append the result to
// results
```
`transform` writes results to the end using assignment, and there is nothing at `results.end()` to assign to.
Hence this is wrong, you have to specify you want things appended, e.g.
```
vector<int> results;
// consider:
// results.reserve(results.size() + values.size());
transform(values.begin(), values.end(), back_inserter(results), transform_int);
```
Internally, `back_inserter` returns an `iterator` which causes `push_back` to be called, so you can do this with anything offering `push_back` (`vector`, `string`, `deque`, `list`).
Similarly, `front_inserter` makes use of `push_front` and works on `deque` and `list`.
We'd be inserting to the front in reverse order, if we don't want the reversed part, do `transform(values.rbegin(), values.rend(), front_inserter(results), transform_int)`.

You can also use an `inserter` on arbitrary locations in the container.
```
vector<int> results;
...
transform(values.begin(),
          values.end(),
          inserter(results, results.begin() + results.size() / 2),
          transform_int);
```

Regardless of `back_inserter`, `front_insert` or `inserter`, each inserts into the destination one object at a time.
Item 5's range member functions suggestions doesn't really apply when we want to use a per-element `transform` insertion.

Item 14's reserve suggestion will bring down the number of reallocates, but the cost of shifting elements per insertion is still there.
Also recall that `reserve` only changes the `capacity` not the `size`: after reserving you still have to use an `inserter` when doing the `transform` above. Doing so without still leads to corrupted container and undefined behavior.

Now if your algorithm wants to overwrite an existing container instead, you don't need an `inserter` but still needs to follow the advice of this item to make sure the destination range is big enough.
Like this:
```
vector<int> values;
vector<int> results;
...
if (results.size() < values.size()) {
    results.resize(values.size());  // make sure results is at least as big as
                                    // values
}
transform(values.begin(), values.end(), results.begin(), transform_int);

// or you can clear results first and then use an inserter like before
results.clear();
results.reserve(values.size());
transform(values.begin(), values.end(), back_inserter(results), transform_int);
```

**Takeaways**
* Whenever you use an algorithm requiring specification of a destination range, make sure that range is big enough already or is increased in size as the algorithm runs (with `ostream_iterator`, or iterators returned by `inserter`, `back_inserter`, `front_inserter`).
