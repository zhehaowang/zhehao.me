### Performance 101 - Chandler Carruth - CppCon 2014

Software is getting slower more rapidly than hardware gets faster

Lower instruction power consumption junk science

Phones, data centers; Compute / watt

Race to sleep; faster program, terminate sooner, is the way to go for optimizing power usage

Java is faster than C++ in some scenarios; tuned scenario, tuned VM. C++, or Java, too, gives you control over performance. In Java such control is usually harder.

Algorithm efficiency (how much work required to get the job done)

Data structures performance (how quickly a program does its work (in time), lighting up all the transistors) 

First and foremost, algorithms
e.g. substring search in nm, KMP, Boyer-Moore

Remember to `vector.reserve`
`unordered_map operator[](key)` does a hash on key and key lookup on every call!

`map<key, unique_ptr<object>>` is a valid pattern (like LocatableWidget)

Imagine function populating a local vector, should you pass a pointer for it to populate, declare a local vector and return by value, or move to return?
Declare a local vector and return by value. In the case of returning the first local variable with only one return statement, compiler is certain to do rvo and copy ellision.
Rvo is faster than output parameters, too. (Input output parameter is a different concern, though)

Always do less work.
Design your API so that your users have the choice to do less work.

Discontiguous data structures are the root of all performance evil.
Say no to linked list. Modern CPU are too fast for spending time waiting for data when traversing a linked list.

std::list traversal, in most cases every traversal is a cache miss.
If you very rarely traverse your data structure, but update your data structure often, linked lists make sense.
What about iterator invalidation: some problems are hard, but still don't use linked list.

What about stacks, queues, maps?
Just use a vector.
Stack, works as-is.
Queues, have a vector, don't erase from front, use an index and don't keep iterators
Maps, speaker claims no real code needs std::map.
unordered_map, does a really useful thing, but required to be implemented with buckets of key-value pairs for each hash entry, and these buckets are linked lists. A good hash table uses open addressing, is flat, uses local probing, and keeps both key and values small.
Deque, speaker avoids std::deque like a plague.

Data structures and algorithms are tightly coupled (in that decision on one usually makes implications about decision of the other): to optimize "performance", you have to keep both in mind and balance between them.

In the real world, worse can be better.
Bubble sort can be the fastest sort (for <64 ints)
Cuckoo hashing: hit collision? Put the new guy in, hash the old guy with a backup hash. Beautiful algorithmic properties, but in practice every probing is a cache miss and pretty bad.

Takeaway
* Both efficiency and performance matter, today more than ever
* C++ helps one control both
* Attend to your algorithms
* Develop pervasive habits of using APIs in a way that avoids wasted work
* Use contiguous, dense, cache-oriented data structures

Move constructor does not invalidate iterators requirement for std::vector is a deal breaker in terms of having small size optimization in std::vector.

What about allocators for maps?
Problem: we have maps on interface boundaries. These are ABI boundaries so they are not templates. Allocator being in type system makes it really hard for a large number of applications. The other problem is having left and right hand side pointers, losing locality (?).
Allocators make it better by giving it better locality, just that the application is usually limited, due to being in type system etc.

What about std::map with a pool? It'd be more pleasant.

What about folly's vector(?) optimization? It does a lot of semantics changes to achieve one benefit: let OS reassign your pointers without moving data around. It works if your address is not part of object's identity (?).

What's needed in std? open-addressed hashtable. Sorted map-like interface on top of vector. Forward list (with tagged pointers). Pooled allocation inside existing data structures. Polymorphic allocators not in type system, but that needs dynamic dispatching, making compiler unable to see and optimize allocations.

B-Trees tend to pick cache-sized nodes, opposed to a binary tree with one-slotted-node. Demonstrating a case where the data structure choice influences the algorithm.

Why does the standard not offer a variety of data structures for developers to pick and choose?
It's better to have reasonable defaults than to allow people to (know how to) pick and choose. Specific domains? Optimize for your use case.
