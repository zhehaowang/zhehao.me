### Use reserve to avoid extra reallocations

STL containers automatically grow to accommodate data you give it (until `max_size`).
For `vector` and `string`, growth is handled by doing a moral equivalent of `realloc` whenever more space is needed.

`realloc` works in 4 parts:
* allocate a new block of memory that is some multiple of the container's current capacity, usually a factor of 2
* copy all the elements from the container's old memory into new memory
* destroy the objects in old memory
* deallocate the old memory

This is expensive and each time it happens, all iterators, pointers and references into the `vector` or `string` are invalidated.
`reserve` allows you the minimum number of reallocations that must be performed, thus avoiding the costs of reallocation and iterator invalidation.

Let's first distinguish some functions (only `vector` and `string` offer this complete set)
* `size()` tells you how many elements are in the container
* `capacity()` tells you how many elements the container can hold in the memory it has already allocated
* `resize(size_t n)` forces the container to change to `n` the number of elements it holds. After call to `resize`, `size` will return `n`. If it used to have more elements the ones at the end will be destroyed. If it used to have fewer default ctor'ed ones will be added to the end
* `reserve(size_t n)` forces the container to change its capacity to at least `n`, provided `n` is no less than the current size. This typically forces a reallocation, because the capacity needs to be increased. (if `n` is smaller than capacity, then the call does nothing.)

```
vector<int> v;
v.reserve(1000);  // do this to minimize reallocations
for (int i = 0; i <= 1000; ++i) {
    v.push_back(i);
}
```

Relationship between size and capacity makes it possible to predict when an insertion will cause a `vector` or `string` to perform a reallocation (and iterators invalidation).

**Takeaways**
* two common ways to use `reserve` to avoid unnecessary reallocations: when you know exactly or approximately how many elements will ultimately end up in your container; or to reserve the maximum space you would ever need then trim.
