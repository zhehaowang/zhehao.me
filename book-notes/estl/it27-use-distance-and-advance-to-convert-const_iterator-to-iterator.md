### Use `distance` and `advance` to convert `const_iterator` to `iterator`

You can't cast from `const_iterator` to `iterator`.
```
deque<in>::iterator i(const_cast<deque<in>::const_iterator>(ci));
// will not compile. same goes for other container types.
// their const_iterator and iterator are just different types.
// similarly static_cast, reinterpret_cast and C-style cast will fail
```

With some implementations of `vector` and `string` who typedefs their iterator to pointer types, `const_cast` will work.
The portability of such code then becomes questionable.

If you have a `const_iterator` and need an `iterator`, there is a safe portable way without messing with the type system.
```
// given
deque<int> d;
deque<int>::const_iterator ci;
// do
deque<int>::iterator i(d.begin());
advance(i, distance<deque<int>::const_iterator>(i, ci));
// create a regular iterator, and advance it to where the const_iterator is.
// note that we explicitly specify distance's template arguments, since
// otherwise it'd think it's working with two different types iterator and
// const_iterator, and won't compile.
```

For random access iterator (`vector`, `string`, `deque`) this is constant. For bidirectional iterators (other stl containers), this is linear.
