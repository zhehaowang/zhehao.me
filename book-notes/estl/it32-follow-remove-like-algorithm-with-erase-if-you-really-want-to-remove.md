### Follow `remove`-like algorithms by `erase` if you really want to remove something

Algorithm `std::remove` can be easy to get wrong.

`remove` takes two iterators and no container, and it's not possible for `remove` to figure out the container it's working with.

The only way to actually eliminate elements from a container is to a member function of that container, usually some form of `erase` (`list` has a few named differently, but they are still member functions).

Therefore `remove` doesn't really remove anything, because it can't.

What `remove` does do is that it moves elements in the range it's given until all the "unremoved" elements are at the front of the range in the same relative order they were originally.
It returns an iterator pointing one past the last "unremoved" element, and this return value is the new logical end of the range.

Note that the standard does not require the removed values at the end to retain their old values, and most implementations (use next to overwrite a removed slot as we go through the container, think of `remove` as performing some form of compaction) don't!
If you want something that retains the old values, you should probably call `partition` instead.

Overwriting values have important repercussions when those values are pointers.

The elements you want to erase are easy to identify: the ones from the new logical end to the real end of the range:
```
vector<int> v;
v.erase(remove(v.begin(), v.end(), 100), v.end());
// with just the remove, v.size() won't change.
// with erase, the elements are actually gone
```

Remove-erase is so common that the above is idiomatic.
`list::remove` does exactly this, and this is the only function in the STL named `remove` that actually eliminates elements from a container.
```
list<int> li;
// ...
li.remove(99); // this actually erases all elements with value 99.
```
Calling `list::remove` is inconsistent in STL, as analogous function in associative containers is called `erase`. (note that it44 points out calling `list::remove` is more efficient than applying erase-remove idiom).

Bear in mind that algorithms like `remove` also can't actually remove: we are looking at `remove_if` and `unique`.
`remove_if` is very similar to `remove`, `unique` removes adjacent repeatedly values.

Similarly, `list::unique` actually removes and is more efficient than erase-unique on the `list`.
