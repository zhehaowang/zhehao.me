# Beware of the illusion of container-independent code

The STL is based on generalization: arrays generalized into containers and parameterized on the types of objects they contain. Functions are generalized into algorithms and parameterized on the types of iterators they use. Pointers are generalized into iterators and parameterized on the type of objects they point to.

Individual container types are generalized into sequence and associative containers; and similar containers are given similar functionality.
Standard contiguous-memory containers offer random-access iterators, standard node-based containers offer bidirectional iterators.

Sequence containers support `push_front` and/or `push_back`, associative containers don't.
Associative containers offer log-time `lower_bound`, `upper_bound` and `equal_range` member functions, but sequence containers don't.

Alas, many programmers try to pursue generalization in that instead of committing to particular types of containers in their code, they try to generalize the notion of a container so that they can e.g. use a `vector` but still preserve the option of replacing it with a `deque` or a `list`, all without changing the code that uses it.
This kind of generalization, although well-intentioned, is almost always misguided.

It rarely makes sense to write code that will work with both sequence and iterative containers, many member functions exist for only one type of the container. Even insert and erase have different signatures in different categories.

Say you want to write code that works with `vector`, `deque` and `list`, you have to code using the intersection of what they offer: you can't use `reserve` or `capacity` or `push_front`, and you give up `operator[]`, and you limit yourself to bidirectional iterators. Meaning you have to steer away from `sort`, `stable_sort`, `partial_sort` and `nth_element`.
If you violate any of these your code will not compile when you swap out containers, and the code that will compile will look insidious.

The main culprit is the different rules for invalidation of iterators, pointers and references that apply to different sequence containers. To write general code you must assume iterator invalidation.

And there's more, to work with C interface you have to use `vector`, you can't instantiate your container with `bool` as it does something different for `vector<bool>` and `list<bool>`.

With all these constraints, your generalization becomes much less applicable. The situation is hardly better if you shoot for general associative containers instead of sequence containers.
So face the truth, it's not worth it. Different containers are different, they each have constraints, strengths and weaknesses that vary in significant ways and are not designed to be interchangeable.

However you'll find yourself needing to change your container someday, and basically inspecting everywhere it's used. You can facilitate such changes with **encapsulation**.
One way is liberal use of `using`s.

Instead of writing this
```
class Widget { ... };
vector<Widget> vw;

Widget bestWidget = ...;
vector<Widget>::iterator i = find(vw.begin(), vw.end(), bestWidget);
    // look up by bestWidget value
```
Write this
```
class Widget { ... };

using WidgetContainer = vector<Widget>;
using WCIterator = WidgetContainer::iterator;

WidgetContainer vw;
Widget bestWidget = ...;

WCIterator i = find(vw.begin(), vw.end(), bestWidget);
```

This makes e.g. adding a custom allocator easier (which does not affect rules for iterator / pointer / reference invalidation).

`using`s encapsulation are purely lexical.
Encapsulate with a class if you want to limit client exposure to the container choices you've made.

To limit code that may require modification if you replace one container type with another, hide the container in a class, and limit the amount of container-specific information visible through the class interface.

E.g. use the following to replace a list
```
class CustomerList {
  private:
    using CustomerContainer = list<Customer>;
    using CCIterator = CustomerContainer::iterator;
    CustomerContainer customers;
  public:
    // limit the amount of list-specific information that's visible through this
    // interface.
};
```
You can't write container-independent code in your implementation of `CustomerList`, but your clients might be able to, hence limiting the scope of change should you need to swap `list` out one day.
