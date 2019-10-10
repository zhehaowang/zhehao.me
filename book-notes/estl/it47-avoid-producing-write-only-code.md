# Item 47. Avoid producing write-only code

Suppose you have a task to get rid of all the elements in a given `vector<int>` whose value is smaller than `x`, except that elements preceding the last occurrence of a value at least as big as `y` should be retained.

This would work:
```
vector<int> v;
int x, y;
...
v.erase(
    // remove elements whose value is smaller than x
    remove_if(
        // find the last occurrence of element at least as big as y
        find_if(
            v.rbegin(),
            v.rend(),
            bind2nd(greater_equal<int>(), y)).base(),
        v.end(),
        bind2nd(less<int>(), x)),
    v.end());
```

This would be hard to maintain or understand, "write-only" code as when you write it it seems straightforward, however, readers will have a hard time decomposing this back into the ideas on which it is based.

This is slightly better
```
// find the start of the range: last occurrence of element at least as big as y
auto rangeBegin = find_if(v.rbegin(),
                          v.rend(),
                          bind2nd(greater_equal<int>(), y)).base();
// remove elements whose value is smaller than x since start
v.erase(
    remove_if(
        rangeBegin,
        v.end(),
        bind2nd(less<int>(), x)),
    v.end());
```

Code is read more often than it is written, and software spends far more time in maintenance than it does in development.

Use the STL well but avoid producing write-only code with it.
