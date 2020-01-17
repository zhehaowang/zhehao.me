### Make copying cheap and correct for objects in containers

Copy in, copy out, that's the STL way.

Once an object is in a container, it's not uncommon for them to be further copied: inserting into or erasing from a `vector`, `string` or `deque` causes existing container elements to be moved (copied) around.
If you use a sorting algorithm, `next_permutation` or `previous_permutation`; `remove`, `unique`, `rotate`, `reverse`, objects will be moved (copied) around.
Copying objects is the STL way.

Copying happens via copy ctor or copy assignment. Built-in types are copied by copying underlying bits.

If you fill a container with objects whose copying is expensive, pushing into a container can be a bottleneck.
If you have objects where "copying" has an unconventional meaning, putting such objects into a container will lead to grief (think `auto_ptr`).

In the presence of inheritance, copying leads to slicing: if you insert objects of derived class into a container of base class objects, the derivedness of the objects will be removed as the objects are copied (via the base class copy ctor) into the container.

```cpp
vector<Widget> vw;
class SpecialWidget : public Widget { ... }
SpecialWidget sw;
vw.push_back(sw);  // sw is copied as a base class object into vw.
```

You can instead create a container of pointers.

STL makes a lot of copies, but it's generally designed to avoid copying objects unnecessarily. In fact it's generally designed to avoid creating objects unnecessarily.

```cpp
Widget w[maxNumWidgets]; // each Widget will be default constructed.
vector<Widget> vw;       // contains 0 Widgets
vw.reserve(10);          // we reserve enough space for 10 Widgets but 0 have
                         // been constructed
```

Compared with C-style arrays STL containers are more civilized. They create only as many copies as you ask for, and do it only when you direct them to. They use a default ctor when you say they should.
Sure they make copies, but they are still a big step up from arrays.

**Takeaways**

* STL copies in, copies out, but does not make unnecessary copies.
* Expensive copy can lead to STL operation being your bottleneck, and conventional copy can lead to wrong behavior in STL operation
* `vector::reserve` only reserves space and does not call any constructor.
