### Understand the proper implementation of `copy_if`

Before C++11 there was not a `copy_if` in STL.

We have `copy`, `copy_backward`, `replace_copy`, `reverse_copy`, `replace_copy_if`, `unique_copy`, `remove_copy`, `rotate_copy`, `remove_copy_if`, `partial_sort_copy_uninitialized_copy`, `ran`.

The new standard has `copy_if`, but if we are to implement our own, one may come up with
```
template <typename InputIterator, typename OutputIterator, typename Predicate>
OutputIterator copy_if(InputIterator begin,
                       InputIterator end,
                       OutputIterator destBegin,
                       Predicate p) {
    // copy everything except where this predicate is not true, this works only
    // with an adaptable function object
    return remove_copy_if(begin, end, destBegin, not1(p));
}
```
The above wouldn't work as `not1` cannot be applied directly to a function pointer: the function pointer must first be passed through `ptr_fun`.
To call this implementation of `copy_if`, you must pass not just a function object, but an adaptable function object.

Here's a correct trivial implementation:
```
template <typename InputIterator, typename OutputIterator, typename Predicate>
OutputIterator copy_if(InputIterator begin,
                       InputIterator end,
                       OutputIterator destBegin,
                       Predicate p) {
    while (begin != end) {
        if (p(*begin)) {
            *destBegin++ = *begin;
        }
        ++begin;
    }
    return destBegin;
}
```
