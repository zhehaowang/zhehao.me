# Pythonic thinking

### Know how to slice sequences

Simplest use for slicing are the built-in types `list`, `str` and `bytes`.
Slicing can be extended to any Python class that implements `__getitem__` and `__setitem__` methods.

Basic syntax is `somelist[start:end]` where `start` is inclusive and `end` is exclusive.

Slicing deals properly with `start` and `end` indices that are beyond the boundaries of the list.
In contrast, accessing an index beyond boundaries directly causes an `IndexError`.

Beware that indexing a list by a negative variable is one of the few situations in which you can get surprising results from slicing, e.g. `a[-0:]` will result in a copy of the original list.

The result of slicing is a whole new list created by copying **the references** of the original list.
Modifying the result of slicing won't affect the original list: the copied reference will be replaced with assigned objects.

```
a     = ['1', '2', '3']
a_ids = [id(x) for x in a]   # [4447883816, 4447883928, 4447884712]
b     = a[:1]
b_ids = [id(x) for x in b]   # [4447883816]
b[0]  = ['4']                # b[0] id now changes to [4413732320], a is not affected
```

When used in assignments, slices will replace the specified range in the original list.
The length of slice assignments don't need to be the same, the list will grow or shrink to accommodate the new values.
If you assign a slice with no start or end indices, you'll replace the entire contents with a copy of what's referenced, instead of allocating a new list.

```
a      = [1, 2, 3, 4, 5]
a[1:3] = [5, 6, 7, 8, 9]  # this will cause `a` to grow to accommodate the new elements

c = a[:]                  # this makes a copy of `a`

d    = a
a[:] = [100, 101]
assert a is b             # a and b are still the same list object
```

**Takeaways**
* Avoid being verbose: don't supply 0 for the `start` index or the length of the sequence for the `end` index.
* Slicing is forgiving of `start` or `end` indices that are out of bounds, making it easy to express slices on the front or back of a sequence.
* Assigning to a list slice will replace that range in the original sequence with what's referenced even if their lengths are different.
