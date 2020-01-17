### Understand how to use `reverse_iterator`'s base `iterator`

Say you have this
```
vector<int> v;
v.reserve(5);

for (int i = 0; i <= 5; ++i) {
    v.push_back(i);
}

vector<int>::reverse_iterator ri = find(v.rbegin(), v.rend(), 3); // ri --> 3
vector<int>::iterator i(ri.base());  // i --> 4
```

If you want to `insert` something before `ri` (since `ri` is `reverse_iterator` this means inserting after 3), `insert` won't let you as it expects a `const_iterator`.
You can give `insert` an `ri.base()` which would do exactly what hoped.

If we want to `erase`, however, we need to erase the element preceding `ri.base()`.

This, however, may not compile for some `vector` and `string` implementations who typedefs `iterator`s to pointers
```
v.erase(--ri.base());
```
Both C and C++ dictate that pointers returned from functions shall not be modified, so when `iterator` is a pointer this won't compile.
You can instead do this, which is portable:
```
...
v.erase((++ri).base())
```

**Takeaways**
* Know that it's not accurate to say `reverse_iterator` base member function returns the corresponding iterator. For insertion purposes it does, but for erasure it does not.
* When erasing knowing a `reverse_iterator`, use the advance the `reverse_iterator` by 1 trick for portability.
