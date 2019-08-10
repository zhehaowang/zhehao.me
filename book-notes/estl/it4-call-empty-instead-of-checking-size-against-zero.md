# Call empty instead of checking size against zero

For any container, writing `if (c.size() == 0)` is essentially equivalent to writing `if (c.empty())`.
`empty()` is typically implemented as an inline function that simply returns whether size is 0.

Why prefer `empty()` then? `empty` is constant time for all standard containers, but for some list implementations, `size` takes linear time.

Why can't list offer a constant-time size? Consider list splicing,

```cpp
list<int> l1;
list<int> l2;

l1.splice(
    l1.end(),
    l2,
    find(l2.begin(), l2.end(), 5),
    find(l2.rbegin(), l2.rend(), 10).base());
// move all nodes in l2 from the first occurrence of 5 through the last
// occurrence of 10 to the end of l1. See it28 for `base()` call.
```

This code won't work unless `l2` contains a `10` somewhere beyond a `5`.
If we assume that's not a problem, instead let's focus on how many elements are in `l1` after the splice? It has as many elements as before plus however many were spliced into it. But how many were sliced into it? Without traversing there is no way to know.

Only `list` offers the ability to constant time `splice` elements from one place to another without copying the data. 

Suppose you want a constant time operation `size` on your list, each list member function, including `splice`, must update the sizes of the lists on which it operates.
The only way for `splice` to update size is to count the number of elements spliced, thus making splice linear.

No matter how you look at it, `size` and `splice` can't both be constant time.
Different `list` implementation can make different tradeoff in `size` vs `splice` efficiency, so your `size` call may be linear.

But no matter what happens, you can't go wrong if you call `empty()` instead of checking `size() == 0`.

**Takeaway**

* When checking if a container is empty, call `empty()` instead of `size() == 0`. `size()` for a container could be linear.
* `list` `splice` and `size` cannot be both implemented in constant time.
