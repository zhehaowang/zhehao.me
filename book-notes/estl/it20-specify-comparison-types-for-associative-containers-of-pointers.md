# Specify comparison types for associative containers of pointers

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
