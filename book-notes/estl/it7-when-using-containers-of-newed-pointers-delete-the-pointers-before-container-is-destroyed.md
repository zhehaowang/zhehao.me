### When using containers of newed pointers, remember to delete the pointers before container is destroyed

The following code leads to a leak:
```
void doSomething() {
    vector<Widget*> vwp;
    for (...) {
        vwp.push_back(new Widget);
    }
    ... // use vwp
    // Widgets are leaked here
}
```
Each of `vwp`'s element is destroyed when `vwp` goes out of the scope, but that's just the pointers: it does not change the fact that the object created with `new` is not destroyed.

You'll want to do
```
void doSomething() {
    vector<Widget*> vwp;
    for (...) {
        vwp.push_back(new Widget);
    }
    ... // use vwp
    for (auto i = vwp.begin(); i != vwp.end(); ++i) {
        delete *i;
    }
}
```

This works but is not exception-safe: it suffers from the same problem as non-RAII objects.

If you want to do `for_each` which takes a `unary_function`, you can do

```
template <typename T>
struct DeleteObject : public unary_function<const T*, void> {
    void operator()(const T* ptr) const {
        delete ptr;
    }
};

void doSomething() {
    ... // as before
    for_each(vwp.begin(), vwp.end(), DeleteObject<Widget>);
}
```

It's annoying that you need to specify `Widget` to instantiate `DeleteObject`.
This redundancy can lead to bugs that are difficult to track down.
E.g. if someone ill-advisedly decides to inherit from string:

```
class SpecialString : public string { ... };
```
This is a risky idea to start with since `string`, like all STL containers, lacks a virtual dtor, and publicly inheriting from classes without virtual destructors is a major C++ no-no.
Still some people do this.

Imagine the following code:
```
void doSomething() {
    deque<SpecialString*> dssp;
    ...
    for_each(dssp.begin(), dssp.end(), DeleteObject<string>());
    // undefined behavior. deletion of a derived object via a base class pointer
    // where there is no virtual dtor
}
```

We can eliminate this redundancy by having compilers deduce the type of pointer being passed to `DeleteObject::operator()`.
All we need to do is move the templatization from `DeleteObject` to its `operator()`:
```
struct DeleteObject {
    template <typename T>
    void operator()(const T* ptr) const {
        delete ptr;
    }
};
```
The downside to this type deduction is we give up the ability to make `DeleteObject` adaptable, though in this case it's difficult to imagine how that would be an issue.

Client code then looks like
```
void doSomething() {
    deque<SpecialString*> dssp;
    ...
    for_each(dssp.begin(), dssp.end(), DeleteObject());
}
```
This is straightforward and type-safe, but still not exception-safe.
This can be addressed in a variety of ways, with the simplest being to have a container of smart pointers instead.

(Side note, don't roll your own `shared_ptr`, the number of subtle ways it can fail is remarkable.)

While a `vector<shared_ptr>` is fine, don't ever do `vector<auto_ptr>`.

**Takeaway**
* When having a container of raw pointers with each pointer new'ed, remember to call delete on them as the container wouldn't do that for you.
* You can call delete in a for loop, have a delete functor and use it with `for_each`, or have a container of `shared_ptr` instead.
