### Know your sorting options

When it comes to sorting, `sort` is not your only option.
If, say, you only need to identify 20 best Widgets, you can do `partial_sort`.

```
bool qualityCompare(const Widget& lhs, const Widget& rhs) {
    ...
}

partial_sort(
    widgets.begin(), widgets.begin() + 20, widgets.end(), qualityCompare);
// after this call the first 20 will be sorted
```

If you only want the 20 best and don't care in what order they are returned, you can use `nth_element`.

```
nth_element(
    widgets.begin(), widgets.begin() + 20, widgets.end(), qualityCompare);
// after this is done, none of the elements in the positions up to n follow the
// element in given sort order, and one of the elements in positions following n
// precede the element at position n in the sort order. Elements in positions
// 1 - 20 won't be sorted.
```

`partial_sort` and `nth_element` order elements with equivalent values any way they want to, and you can't control this aspect of their behavior.

For a full `sort`, you have more control and can do `stable_sort`.

`nth_element` can also be used to find the median element or a particular percentile. E.g.
```
vector<Widget>::iterator begin(widgets.begin());
vector<Widget>::iterator end(widgets.end());

vector<Widget>::iterator goalPosition = begin + widget.size() / 2;
nth_element(begin, goalPosition, end, qualityCompare);
// goalPosition now points to the Widget with median quality

// similarly, 75-percentile
vector<Widget>::iterator goalPosition = begin + widget.size() / 4;

```

Now suppose you want to identify all the Widgets with a quality rating 1 or 2.
You could do a full sort but that would be wasteful.
You could instead do a `partition`:
```
bool hasAcceptableQuality(const Widget& w) {
    // return whether w has a quality rating of 2 or better
}

vector<Widget>::iterator goodEnd = partition(
    widgets.begin(), widgets().end, hasAcceptableQuality);
// move all Widgets satisfying hasAcceptableQuality to the front of Widgets, and
// return an iterator to the first iterator that isn't satisfactory.
```

If we want to maintain relative position of equivalent Widgets, do `stable_partition` instead.

`sort`, `stable_sort`, `partial_sort`, `nth_element` require random access iterators, so they may be only applicable to `vector`, `string`, `deque` and `array`.

Doesn't make sense to sort a `map` or `set`, and for `list` it comes with a member `list::sort`, which is a stable sort.

`partition` and `stable_partition` only require bidirectional iterators. You can use these with any standard sequence containers.

In addition, you can keep things sorted at all times by storing your data in a standard associative container, or a `priority_queue`.

In general, `partition` is faster / uses less resource than `partial_sort` than `stable_partition` than `sort` than `nth_element` than `stable_sort`.

The general advice is to make your selection based on what you need to accomplish, not on performance considerations.
Choose a sort algorithm that does only what you need to do expresses what you need to do more clearly, and could also be the most efficient way to accomplish it with STL.
