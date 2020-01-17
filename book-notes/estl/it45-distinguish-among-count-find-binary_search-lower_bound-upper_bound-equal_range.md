### Item 45. Distinguish among `count`, `find`, `binary_search`, `lower_bound`, `upper_bound` and `equal_range`

Suppose you want to look up something from a container or a pair of iterators.

All of the algorithms in the title can achieve this. How to choose?

If you are given a pair of iterators, first consideration is if the range is sorted.
If so, `binary_search`, `lower_bound`, `upper_bound` and `equal_range` are log-time.
If not, try linear ones like `count`, `count_if`, `find`, `find_if`.

`count` answers the question how many copies of something is in there?
`find` answers the question if something is in and if so where is it?

To just test for something exists or not, you can do
```
if (count(lw.begin(), lw.end(), w)) { ... } // or
if (find(lw.begin(), lw.end(), w) != lw.end()) { ... }
```
`count` seems simpler but less efficient: it keeps going after finding something.

When you need to know where that something is, use `find`.

When dealing with sorted range, we shift from using equality to decide if two are the same to using equivalence: `count` and `find` use equality, `binary_search`, `lower_bound`, `upper_bound` and `equal_range` use equivalence.

Use `binary_search` to answer the question if something is in there: it returns just a `bool`.

To know where it is, you can do `equal_range` or `lower_bound`.

`lower_bound` returns an iterator pointing to either the first copy of that value if it's found, or to the proper insertion location for that value if it's not found.
"is it there? if so, where is the first copy, and if not, where would it go?"

Most programmers do `lower_bound` like this
```
vector<Widget>::iterator i = lower_bound(vw.begin(), vw.end(), w);
if (i != vw.end() && *i == w) { ... }
```
This has a bug as `lower_bound` searches using equivalence but this tests for equality.
When equality and equivalence can yield different results, this is problematic.

If you want to do it right, be sure to use the same comparison function `lower_bound` uses.

`equal_range` can be easier: it returns a pair of iterators the first equal to the iterator `lower_bound` would return, the second equal to the one `upper_bound` would return (i.e. one-past-the-end iterator for the range of values equivalent to the one searched for).

Two observations about `equal_range`: if the returned two iterators are the same, that means the range of objects is empty: the value isn't found.
Answer `is it there` with `equal_range` like this"
```
vector<Widget> vw;            // sorted
pair<vector<Widget>::iterator, vector<Widget>::iterator> p = equal_range(
    vw.begin(), vw.end(), w);
if (p.first != p.second) {
    // in!
} else {
    // not in!
}
```
This code only uses equivalence so it is always correct.

Distance (`distance(p.first, p.second)`) between the two returned iterators is the number of objects in range.

Suppose we have a `vector` `v` of timestamp and we want to eliminate everything before a timestamp `t`
```
v.erase(v.begin(), lower_bound(v.begin(), v.end(), t));
```
If we want to remove elements before and including `t`
```
v.erase(v.begin(), upper_bound(v.begin(), v.end(), t));
```

`upper_bound` is also useful for inserting in order.
Imagine we have this
```
class Person {
  public:
    const string& name() const;
};

struct PersonNameLess : public binary_function<Person, Person, bool> {
    bool operator()(const Person& lhs, const Person& rhs) const {
        return lhs.name() < rhs.name();
    }
};

list<Person> lp;
lp.sort(PersonNameLess());
```
To keep the list sorted the way we desire, can do this
```
Person newPerson;
lp.insert(
    upper_bound(lp.begin(), lp.end(), newPerson, PersonNameLess()),
    newPerson);
```
This is fine but note that since we are working with a `list` this is linear time instead of logarithmic. (it34 notes this performs logarithmic number of comparisons but the lookup takes linear time)


Now if you are given a container instead of a pair of iterators, you should distinguish between sequence and associative containers.

For standard sorted associative containers (`set`, `multiset`, `map`, `multimap`), they offer member functions that are typically better choices than STL algorithms.
It44 would suggest we use member versions of `count`, `find`, `equal_range`, `lower_bound` or `upper_bound`.
`binary_search` does not have a member version. Use `count` for `set` and `map`, and `find` for `multiset` and `multimap`, if you care about efficiency.

As a general rule of thumb, choose the algorithm or member function that offers you the behavior and performance you need and that requires the least amount of work when you call it.
