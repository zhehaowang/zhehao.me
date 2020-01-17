### Use swap trick to trim excessive capacity

Erasing from a `vector` does not reduce its capacity.

Sometimes you want to shrink to fit, say you have a vector `v`, you can trim away excessive capacity using
```
vector<int>(v).swap(v);
```
This creates a temporary vector that's a copy of `v`, `vector`'s copy ctor does the work, beware that the copy ctor only allocates as much as is needed for the elements being copied, so this temporary vector has no extra capacity.
We then swap the data in the temporary vector with that in `v`, and temporary now has all the extra capacity.
After we are done the temporary is destroyed.

The same trick is applicable to strings:
```
string s;
... // make a large string, then erase of its characters
string(s).swap(s); // shrink-to-fit using swap
```

Note that the implementer may decide to give the temporary `string` / `vector` a minimum capacity or a capacity of power of 2. But this way to shrink excessive capacity is still pretty good.s

Swap trick can also be used to clear a container: swap with an empty container
```
string().swap(s);
```

Swap the contents of two containers also swaps their iterators, pointers and references.
Iterators, pointers, and references that used to point to elements in one container remain valid and point to the same elements, but in the other container, after swap. (_how?_)
