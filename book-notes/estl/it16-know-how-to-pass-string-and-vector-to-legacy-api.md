### Know how to pass `string` and `vector` to legacy API

C API exists and will exist.

Given `vector<int> v` and an API `void doSomething(const int* pInts, size_t numInts)`, we do pass like this `doSomething(&v[0], v.size())`.

The only sticking point is if `v` is empty, `&v[0]` yields undefined results.

To be safe, instead do
```
if (!v.empty()) {
    doSomething(&v[0], v.size());
}
```

Some say you can pass `v.begin()` instead of `&v[0]`, counting on iterators always being pointers. This is often true, but not always, and you should never rely on it.

If you are determined to use `v.begin()` for some reason, use `&*v.begin()`.

The approach to getting a pointer to container data that works for vectors isn't reliable for strings, because the data for strings are not guaranteed to be stored in contiguous memory, and internal representation of a string is not guaranteed to end with a null character. That's why we use `c_str` member function in string's case.

Something like `void doSomething(const char* pString)` can be passed `doSomething(s.c_str())`.
This works even if string is of length 0.
This also works if the string has embedded nulls, if it does `doSomething` is likely to interpret the first embedded null as the end of the string, `string` objects don't care if they contain null characters, but `char*`-based C APIs do.

Note again in both our examples above, the vector or string data are being passed to an API that will read it, not modify it.
For strings, there is no guarantee that `c_str` yields a pointer to the internal representation of the string data: it could return a pointer to an unmodifiable copy of the string's data formatted correctly for C (no contemporary library implementation does this, to the author's knowledge).
For `vetor`, you have a little more flexibility. if you pass `v` to a C API that modifies `v`'s elements, that's typically ok, but the called routine must not attempt to change the number of elements in a `vetor`: e.g it must not create new elements in vector's unused capacity, if it does `v.size()` will then yield wrong answer; even worse if you do this to a vector whose size is at its capacity.

If you've a vector that you'd like to initialize with elements from a C API, you can also take advantage of underlying layout compatibility.
```
// C API that fills the given pArray
size_t fillArray(double *pArray, size_t arraySize);

vector<double> vd(maxNumDoubles);
// create a vector whose size is maxNumDoubles

vd.resize(fillArray(&vd[0], vd.size()));
// have your C API fill the vector, then resize the vector to the number of
// elements filled

// or when target is a string (this copies from the vector, so you can target
// a deque, list, set, etc)
size_t fillString(char* pArray, size_t arraySize);

vector<char> vc(maxNumChars);
size_t charsWritten = fillString(&vc[0], vc.size());
string s(vc.begin(), vc.begin() + charsWritten);

// similarly, to pass your data in a set / list to a C API expecting pointer and
// size, you can copy the elements to a vector first, then pass the vector as
// shown above.

// you could also copy into a dynamically allocated array, but item 13 explains
// why you should prefer vectors.
```
