# Choose your containers with care

### Know your options

* STL sequence containers: `vector`, `string`, `deque`, `list`
* STL associative containers: `set`, `multiset`, `map`, `multimap`
* non standard sequence containers: `slist` singly linked list, `rope` a heavy-duty string
* Hash based associative containers: `unordered_set`, `unordered_map`, `unordered_multiset`, `unordered_multimap`
* Standard non-STL containers: `array`, `bitset`, `valarray`, `stack`, `queue`, `priority_queue`

* algorithmic complexity
* contiguous-memory containers (`vector`, `string`, `deque`, `rope`, stores multiple elements per chunk of dynamically allocated memory; random insertion and deletion will have to cause shifts) vs node-based containers (`list`, `slist`, associative containers (and hash based ones to a degree) stores a single element per chunk of dynamically allocated memory, element values need not be moved around when inserting, erasing)

Things to consider:
* do you need to be able to randomly insert? If so, you need a sequence container, associative containers won't do
* do you care about element ordering? If so, hash-based containers won't do.
* must the container be part of standard
* what category of iterator do you require? If random access iterator required, `vector`, `deque`, `string`, `rope`. If bidirectional required, avoid `slist` and one common implementation of hash based containers.
* is it important to avoid shifting elements when insertion / erasure takes place? If so, contiguous memory containers won't do.
* does the data layout need to be compatible with C? If so, you are limited to `vector`
* is look up speed critical? If so, consider hash, sorted vectors, and standard associative containers, probably in that order
* do you mind if underlying container uses reference counting? If so, you'll want to steer clear of `string` because many `string` implementations use reference counting. Also steer clear of `rope`. To represent your string, consider a `vector<char>`
* do you need transactional semantics for insertions and erasures? (do you require the ability to reliably roll back insertions and erasures?) If so, use a node based container. If you need transactional semantics for multiple-element insertions (the range form), use `list` as it's the only one from standard that offers transactional semantics for multiple-element insertions. Transactional semantics are particularly important for exception-safe code.
* do you need to minimize iterator, pointer and reference invalidation? If so, use node-based containers, because insertions and erasures on such containers never invalidate iterators, pointers or references (unless they point to an element you are erasing). In general, insertions and erasures on contiguous-memory containers may invalidate all iterators, pointers and references into that container
* would it be helpful to have a sequence container with random access iterators where pointers and references to the data are not invalidated as long as nothing is erased and insertions take place only at the ends of the container? `deque` works for this exact case.

These questions are hardly the end of the matter. E.g. this doesn't take into account different memory allocation strategies of different container types.

**Takeaways**

* Container choice should not only base on algorithmic complexity. Memory access pattern, ordering, iteration support, layout, transactional semantic, etc are often important as well.
* Know your options. Contiguous vs node-based. Sequence vs associative. Hash-based associative vs order-based associative.
