### Be aware of allocator conventions and restrictions

Allocators were originally introduced to allow library designers to ignore distinctions between near and far pointers in certain 16-bit OS (e.g. DOS).
They were then designed to facilitate the development of memory managers that are full-fledged objects, but it turned out that led to degradations in parts of the STL.

The standardization committee added wording to the standard that emasculated allocators as objects, yet simultaneously expressed the hope that they would suffer no loss of potency from operation.

Like `operator new` and `operator new[]`, STL allocators are responsible for allocating and deallocating raw memory, but an allocator's client interface bears little resemblance to those, or even `malloc`.
Finally, most of the standard containers never ask their associated allocator for memory. The end result is that allocators are weird.

Start by knowing what allocators aren't meant for.

In the standard, the default allocator for `T`, `allocator<T>`, offers typedef `allocator<T>::pointer` and `allocator<T>::reference`, and it is expected that user-defined allocators will provide these typedefs, too.

This is suspect as there is no way to fake a reference in C++: doing so requires the ability to overload `operator.`, which is not permitted.
In addition, creating objects that act like references is an example of the use of proxy objects, and proxy objects lead to a number of problems (item 30 of more effective c++).

It's not proxy objects causing problems in the STL allocator, it's that standard allows library implementers to assume that every allocator's pointer typedef is a synonym for `T*` and every reference typedef is the same as `T&`: library implementers may ignore the typedefs and use raw pointers and references directly.

There is another quirk: standard says that an implementation of the STL is permitted to assume that all allocators objects of the same type are equivalent and always compare equal.
This doesn't sound awful, consider this code:
```
template <typename T>
class SpecialAllocator { ... }; // user-defined allocator template

typedef SpecialAllocator<Widget> SAW; // SpecialAllocator for Widgets

list<Widget, SAW> l1;
list<Widget, SAW> l2;
...
l1.splice(l1.begin(), l2);  // move l2's nodes to the front of l1
```

Note that when splicing a list nothing is copied, a few pointers are adjusted such that splicing is fast and exception-safe.
When `l1` is destroyed, it must destroy all its nodes and deallocate their memory, and because now it contains nodes that were originally part of `l2`, `l1`'s allocator must deallocate nodes that were allocated by `l2`'s allocator.
This is why standard wants different allocator objects of the same type to compare equal, without which slicing would be difficult to implement.
(Remember that an efficient slicing also impact `size` function of containers.)

This is a very draconian restriction placed on STL allocators: they must not have states; they can't have any non-static data members, at least not any that affect their behavior; you can't have two allocator objects of the same type allocate from different heap space.

Note that the above is a runtime issue: allocator with state will compile just fine, but they may not run the way you expect them to.
Standard actually says implementors are encouraged to supply libraries that support non-equal instances, in which case the semantics are implementation-defined, but this offers nothing to a user who cares about portability.

If we compare the interface of `operator new` with `allocate<T>::allocate`:
```
void* operator new(size_t bytes);
pointer allocator<T>::allocate(size_type numObjects);
// "pointer" is a typedef that's virtually always T*
```
There's nothing wrong with this discrepancy, just that this inconsistency complicates development.
Note the return type discrepancy: the `pointer` type returned doesn't really point to a `T` object, because no `T` object has yet been constructed. The assumption is that the caller will eventually construct one or more `T` objects in the memory it returns (possibly via `allocator<T>::construct`, `uninitialized_fill`, or some other forms `raw_storage_iterators`), though in the case of `vector::reserve` or `string::reserve`, this may never happen.

The final curiosity of STL allocators is that most of the standard containers never make a single call to the allocators with which they are instantiated.
```
list<int> l; // same as list<int, allocator<int> >. allocator<int> is never
             // asked to allocate memory.
set<Widget, SAW> s; // no SAW will ever allocate memory
```
This oddity is true for `list`, `set`, `multiset`, and `multimap`, because these are node-based containers.
When we need to insert a new node, we need memory for `ListNode` that contains `T` rather than an allocator who allocates `T`.
std provides a way to do this:
```
template <typename T>
class allocator {
  public:
    template <typename U>
    struct rebind {
        typedef allocator<U> other;
    };
};
// the standard allocator is declared like this but it could be a user-written
// allocator template, too
```
The type of allocator for `ListNode` that corresponds to the allocator we have for `T` is `Allocator::rebind<ListNode>::other`.

Every allocator template `A` is expected to have a nested struct template called rebind, which takes a single type argument `U`, and defines a typedef, `other`, which is a name for `A<U>`. `list<T>` can then use `Allocator::rebind<ListNode>::other` to allocate `ListNode`, where `Allocator` is its allocator for `T`.

Here's the list of things you need to remember if you ever write an STL allocator:
* Make your allocator a template, with type parameter `T` representing the type of objects for which you are allocating memory
* Provide typedefs `pointer` `reference`, but always have pointer be `T*`, and reference be `T&`
* Never give your allocators per-object state. In general, allocators should have no non-static members
* Remember that an allocator's `allocate` method are passed the number of objects for which memory is required, not the number of bytes needed. Also remember these functions return `T*` via `pointer` typedef, even though no `T` objects have yet been constructed
* Be sure to provide the nested rebind template on which standard containers depend

Most of this is boilerplate code that you don't want to write yourself. Start by looking at a good sample, then tinker with what you need to customize: notably `allocate` and `deallocate`.
