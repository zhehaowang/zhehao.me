### Make functor classes adaptable

Suppose I have a list of `Widget`s and a function to determine if such a pointer identifies an interesting `Widget`.
To find the first pointer to an interesting `Widget` is easy.
```
list<Widget*> widgetPtrs;
bool isInteresting(const Widget *pw);

list<Widget*>::iterator i = find_if(
    widgetPtrs.begin(), widgetPtrs.end(), isInteresting);

if (i != widgetPtrs.end()) { ... }
```

To find the first pointer to a `Widget` that is not interesting is not so straightforward.
```
list<Widget*>::iterator i = find_if(
    widgetPtrs.begin(), widgetPtrs.end(), not1(ptr_fun(isInteresting)));
```

The only thing `ptr_fun` does is make some typedefs available.
These typedefs are required by `not1`.
If you don't care about adapting, with / without `ptr_fun` are the same, no runtime cost whatsoever.
If you get confused when to use them and when not to, consider using it every time you pass a function to an STL algorithm.

Each of the four standard function adapters (`not1`, `not2`, `bind1st`, `bind2nd`) requires the existence of certain typedefs.
Function objects that provide the necessary typedefs are said to be adaptable.

Making your function objects adaptable costs you nothing, and could buy your clients a world of convenience.

The typedefs in question are `argument_type`, `first_argument_type`, `second_argument_type` and `result_type`.

Unless you are writing your own adapters, you don't need to know about these typedefs, as the conventional way to get them is to inherit from a base class.

For functor classes whose `operator()` takes one argument, we can inherit from `std::unary_function`.
For those taking two we can inherit from `std::binary_function`.

Example:
```
template<typename T>
class MeetsThreshold : public std::unary_function<Widget, bool> {
  private:
    const T threshold;
  public:
    MeetsThreshold(const T& threshold);
    bool operator()(const Widget&) const;
};

struct WidgetNameCompare : std::binary_function<Widget, Widget, bool> {
  bool operator()(const Widget& lhs, const Widget& rhs) const;
};
```

Note that even though our argument type is a reference, when instantiating `unary_function` we take away the referenceness.
For pointers they should stay the same.

Inheriting from these yields adaptable function objects, which allows us to do the likes of these:
```
list<Widget> widgets;
...
list<Widget>::reverse_iterator i1 = find_if(widgets.rbegin(), widgets.rend(), not1(MeetsThreshold<int>(10)));

Widget w;
list<Widget>::iterator i2 = find_if(widgets.begin(), widgets.end(), bind2nd(WidgetNameCompare(), w));
```

STL implicitly assumes that each functor class only has one `operator()` function.
Don't overload your function object's `operator()` if you want adaptability gained from inheritance in this item.

This chapter might be deprecated as lambda in newer standards is probably a good replacement for old use cases involving adapting with `bind`, etc.
