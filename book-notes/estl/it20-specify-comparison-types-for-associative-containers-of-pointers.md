### Specify comparison types for associative containers of pointers

Imagine you've a `set<string*> s` with `s.insert(new string("abc"))`, when iterating over `s` you should not expect your string to come out in alphabetical order as `set` sorts it on `less<string*>` as opposed to `less<string>`.

`set<string*> s` is shorthand for `set<string*, less<string*>, allocator<string*>> s`.

If you want the `set` to sort on alphabetical order, you can have a custom comparator like this
```
struct StringPtrLess : public binary_function<const string*, const string*, bool> {
    bool operator()(const string* ps1, const string* ps2) {
        return *ps1 < *ps2;
    }
};

set<string*, StringPtrLess> s;

// say you have this set and now wants to print all elements, you could do

void print(const string* ps) {
    cout << *ps << "\n";
}

for_each(s.begin(), s.end(), print);

// or a generic deference functor class

struct Dereference {
    template <typename T>
    const T& operator()(const T* ptr) const {
        return *ptr;
    }
};

transform(s.begin(), s.end(), ostream_iterator<string>(cout, "\n"), Dereference());
```

The point is that any time you create a standard associative container of pointers, you must bear in mind that the container will be sorted by the value of the pointers.
That is rarely what you want, so you'll almost always want to create your own functor class to serve as a comparison type.
Note that it's a comparison type, not simply a comparison function, since `set`'s template expects a type, not a comparison function.

If most of the time you are just going to dereference and compare, you can have a template like this:
```
struct DereferenceLess {
    template <typename PtrType>
    bool operator()(PtrType p1, PtrType p2) const {
        return *p1 < *p2;
    }
};

// we can then write
set<string*, DereferenceLess> ssp;
```

Same goes for associative containers of smart pointers / iterators.
