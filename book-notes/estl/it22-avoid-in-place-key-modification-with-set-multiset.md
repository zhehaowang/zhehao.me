# Avoid in-place key modification in set and multiset

These are sorted by the element, and proper behavior relies on the container being sorted. 
Changing an element in-place may break the sortedness of the container.

This is automatically enforced for `map` and `multimap`, as elements in both have the type `pair<const K, V>`.
To change `K` you'll have to do a `const_cast`.

In-place modification of elements is possible for `set` and `multiset`.
Why don't `set` and `multiset` hold a `const V`?
Suppose we have this `Employee` class:
```
class Employee {
  public:
    ...
    const string& name() const;
    void setName(const string& name);
    ...
    int id() const;
};
```

When holding a `set` of `Employee`, we want to sort on the `id` of each.
We want to change the `name` of `Employee` in a `set`, and this would not affect the sortedness of `set<Employee>`.
`const V` blocks us from doing that.

Doesn't the same argument hold for `map` and `multimap`?
Maybe, but that's not what the standardization committee thought. To do so you would then need to `const_cast`.

So the point of this item is really to not to modify in-place the key part of an element (one that would affect the sortedness of the container).

Some standard implementation have `operator*` for a `set<T>::iterator` return a `const T&`, such implementation is arguably legal as the standard is inconsistent in this area.
So if you value portability, assume elements of a `set` cannot be modified. If you don't, when modifying an element, don't modify its key parts.

If you do go ahead with a cast, remember to cast it to a reference.
```
MySet::Iterator i = s.find(id);
if (i != se.end()) {
    const_cast<Employee&>(*i).setTitle("a");
}
```
Don't do this
```
static_cast<Employee>(*i).setTitle("a");
// or its equivalent
((Employee)(*i)).setTitle("a");
```
In both these cases they modify a temporary copy.

casts should be avoided.
If you want to change an element in a `set`, `multiset`, `map`, `multimap` in a way that always works and is safe, find the one you want to modify, make a copy, remove the original, modify the copy, and insert the copy (consider using the iterator you get from find as a hint to bring complexity from logarithmic to constant).
Like
```
MySet s;
Employee eid;
...
MySet::iterator i = s.find(eid);
if (i != s.end()) {
    Employee e(*i);
    s.erase(i++);    // it9
    e.setTitle("a");
    s.insert(i, e);  // insertion with hint
}
```

With `set` and `multiset`, remember that if you perform any in-place modification on container elements, you are responsible for making sure the container remains sorted.
