# Pythonic Thinking

### Avoid using `start`, `end` and `stride` in a single slice

Python has syntax `somelist[start:end:stride]`. E.g. stride makes it easy to group by even and odd indexes in a list.

The problem is that stride often cause unexpected behavior that can introduce bugs, e.g. `x[::-1]` on unicode characters encoded as UTF-8 byte strings will break if you decode them, while this works on ascii characters.

Using `stride` with `start` or `end` in a slice can be confusing.
If you need to use `stride` and `start` or `end`, consider doing it in two statements.

**Takeaways**
* Specifying `start`, `end` and `stride` in a slice can be confusing.
* Prefer using prositive `stride` value in slices without `start` or `end` indexes, avoid negative `stride` value if possible.
* Avoid using `start`, `end` abd `stride` in a single slice. If you need all three parameters, consider doing two assignments (one to slice another to stride) or using `islice` from `itertools`.
