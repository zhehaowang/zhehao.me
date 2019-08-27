# Item 35: implement simple case insensitive string comparisons via mismatch or lexicographical compare

Using stl to perform case-insensitive string comparisons can be easy or hard.
Implement what `strcmp` does is easy, if you want to be able to handle other languages except English is hard.
This item tackles the easy problem as the hard one involves no more STL.

```
// case insensitive compare chars c1 and c2. Return -1 if c1 < c2, 1 if c1 > c2,
// 
int ciCharCompare(char c1, char c2) {
    int lc1 = tolower(static_cast<unsigned char>(c1));
    int lc1 = tolower(static_cast<unsigned char>(c2));

    if (lc1 < lc2) return -1;
    if (lc1 > lc2) return 1;
    return 0;
}

// note that tolower() in <cctype> takes int returns int, and the parameter type
// needs to be represented as unsigned char, hence the cast and storing return
// value as int
```

```
int ciStringCompareImpl(const string& s1, const string& s2) {
    pair<string::const_iterator, string::const_iterator> p = mismatch(
        s1.begin(), s1.end(), s2.begin(), not2(ptr_fun(ciCharCompare)));
    if (p.first == s1.end()) {
        if (p.second == s2.end()) {
            return 0;
        } else {
            return -1;
        }
    }

    return ciCharCompare(*p.first, *p.second);
}

// note that this requires s1 to be no longer than s2, to guarantee that we
// expose this interface that calls into the above

int ciStringCompare(const string& s1, const string& s2) {
    if (s1.size() <= s2.size()) {
        return ciStringCompareImpl(s1, s2);
    } else {
        return -ciStringCompareImpl(s2, s1);
    }
}

```

`mismatch` performs the heavy lifting.
It returns a pair of iterators indicating the locations in the ranges where corresponding characters first fail to match.

Note the `not2(ptr_fun(ciCharCompare))`, which is responsible for returning true when the characters match.
Since `ciCharCompare` returns -1, 1, or 0, we'll need to apply `not` on its return value.

Another useful thing to have is a function predicate that returns if case-insensitive `s1 < s2`.
We could do a `lexicographical_compare`.
```
bool ciCharLess(char c1, char c2) {
    return tolower(static_cast<unsigned char>(c1)) <
           tolower(static_cast<unsigned char>(c2));
}

bool ciStringCompare(const string& s1, const string& s2) {
    return lexicographical_compare(
        s1.begin(), s1.end(), s2.begin(), s2.end(), ciCharLess);
}
```

`lexicographical_compare` is a generalized version of `strcmp`: `strcmp` only works with character arrays, and `lexicographical_compare` works with ranges of any type. (also `strcmp` returns -1, 0, 1 for different states and `lexicographical_compare` returns true, false as a predicate to indicate less than as defined by a user-given predicate)

Case insensitive string comparison functions are also widely available as nonstandard extensions to the C library.
They typically have names like `strcmpi` or `stricmp` and typically offer no support for non-English characters.
You may be able to replace `ciCharCompare` with just `stricmp(s1.c_str(), s2.c_str())`.

`stricmp`, being optimized to do one thing, can often run much faster on long strings than the general-purpose algorithms `mismatch` and `lexicographical_compare`. If this is important to you, you very well should consider using `stricmp` and potentially sacrifice some portability.
