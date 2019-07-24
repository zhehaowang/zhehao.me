# Never create containers of `auto_ptr`

Code attempting to create containers of `auto_ptr` should not compile, the standardization committee took trouble to prohibit such.
But many work with STL platforms that don't reject such, or even see them as an easy approach to resource management.

When you copy an `auto_ptr`, ownership of the object pointed to by the `auto_ptr` is transferred to the copying `auto_ptr`, and the copied `auto_ptr` is set to null.
Copying an `auto_ptr` actually changes its value!

```
auto_ptr<Widget> pw1 (new Widget); // pwl1points to a Widget
auto_ptr<Widget> pw2(pw1); // pw2 points to pw1's Widget
                           // pw1 is set to NULL. (Ownership
                           // of the Widget is transferred
                           // from pw1 to pw2.)
pw1 = pw2; // pw1 now points to the Widget
           // again; pw2 is set to NULL
```

This can lead to very surprising behavior in STL. Consider this code.
```
bool widgetAPCompare(const auto_ptr<Widget>& lhs,
                     const auto_ptr<Widget>& rhs) {
    return *lhs < *rhs;
    // for this example, assume that operator< exists for Widgets
}

vector<auto_ptr<Widget> > widgets;
// create a vector and then fill it with auto_ptrs to Widgets;
// remember that this should not compile!

sort(widgets.begin(), widgets.end(), widgetAPCompare);
```
One or more of the `auto_ptr`s in widgets may have been set to `NULL` during the sort.
Imagine our `sort` uses quicksort, in it we may make a copy of an element as `pivot`. This is problematic as the `auto_ptr` in the vector is now `NULL`, and when `pivot` goes out of scope, the element is destroyed.
By the time `sort` returns, the contents of the vector will have changed, and at least one element would have been deleted.

This is a nasty trap people can fall into, hence standard forbids usage of `auto_ptr` with STL containers.
You can use `unique_ptr`, or `shared_ptr`.

**Takeaway**
* Don't use STL containers of `auto_ptr`
* `auto_ptr` "copy" is actually a move
