### Choose carefully between `map::operator[]` and `map::insert` when efficiency is important

```
class Widget {
  public:
    Widget();
    Widget(double weight);

    Widget& operator=(double weight);
};

map<int, Widget> m;
m[1] = 1.5;
m[2] = 3;

...
```
This could incur a considerable performance hit, as `map::operator[]` is designed to facilitate add or update, it need to return a reference to a `Widget`, which it doesn't have so we have to default construct that `Widget` first, then call the copy assignment operator.
`m[1] = 1.5` is equivalent to
```
pair<MyMap::iterator, bool> result = m.insert(MyMap::value_tpye(1, Widget()));
result.first->second = 1.5;
```
We might as well do
```
m.insert(MyMap::value_type(1, 1.5));
// saves a default ctor, dtor the temporary and calling assignment operator
```

Now when we do update instead:
```
m[k] = v; // vs
m.insert(MyMap::value_type(k, v)).first->second = v;
```
From efficiency perspective, the `insert` version needs to ctor and dtor a `pair`, while `operator[]` doesn't do pairs.

STL provides no `efficientAddOrUpdate` but we can write one ourselves:
```
template <typename MapType, typename KeyArgType, typename ValueArgType>

typename MapType::iterator
efficientAddOrUpdate(MapType &m, const KeyArgType &k, const ValueArgType &v) {
    typename MapType::iterator lb = m.lower_bound(k);
    if (lb != m.end() && !(m.key_comp()(k, lb->first))) { // be sure to test for
        lb->second = v;                                   // equivalence
        return lb;
    } else {
        typedef typename MapType::value_type MapValueType;
        return m.insert(lb, MapValueType(k, v));  // standard guarantees if hint
                                                  // is correct, insert happens
                                                  // in constant time
    }
}
```
Note that `KeyArgType` and `ValueArgType` don't have to be the types stored in the `map`: they only need to be convertible to the types stored in the `map`.
We could instead do `MapType::key_type` and `MapType::mapped_type` but that might force unnecessary type conversions.

**Takeaways**
* When efficiency is important, `map::operator[]` is proper for find and replace, and `map::insert` is proper for inserting a new element.
