### Hybrid data structure - Chandler Carruth - CppCon 2016

Vector, sets and maps

##### Vector is all you need: queues, priority queues, binary heap

Why not std::vector: std asks the iterator to not be invalidated if you move the vector from one location to another.
This means a small-size optimization is completely off the table.
std::basic_string with character traits actually uses small-size optimizations.

SmallVector<T, Size> deriving from SmallVectorImpl<T>: (child passing in the buffer through the parent's ctor interface) claim is that when we have input and output parameters (or just output parameters) who are small vectors, we can remove the size from the array type completely. (let clients pick sizes?)
    
##### SmallDenseSet

Open addressing, flat in memory, quadratic probing

All traits in one trait object, with getEmptyValue() and getTombstoneValue(), and getHash() and compare()

##### SmallDenseMap

Similar with Set, with a pair as value.
Note that tombstone doesn't need to be a full pair.

##### Discussion, why not allocators

They absolutely work, but
* it gets awkward in interface boundaries: size needs to by synced across callers.
* we use auto often to denote the return value, but if we go with allocators we can potentially return values allocated on stack

We get value semantics back for the container, even if it's a small-size-optimized container.

* Small containers are best when small!
* Give large objects address identity: if we **allocate stably**, the address of the object can be used as a stand-in for the object itself.

SmallVector<unique_ptr<BigObject>, 4> Objects vs list: list has two pointers! forward_list is hard to mutate!
This is faster in iteration (if it's not iteration you care about we should probably worry about something other than containers): processor optimized to find out what memory you are going to use next. SmallVector allows that, list doesn't and will result in more cache misses.
    
(thinking back on our `SmallVector<MegaOrder*>`, it should actually benefit from what Chandler claims. But know that this is for the iteration use case, which may or may not be prominent in btrade use case)

BumpPtrAllocator allocating a slab, stable addresses and high locality for large objects

If pointers are too large, use an index

##### aggressively pack bits

4 free zeroes bits in the pointer with llvm.
With each pointer, we can piggyback 4 bits of irrelevant data.