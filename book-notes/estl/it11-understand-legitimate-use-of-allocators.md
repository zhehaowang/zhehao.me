### Understand the legitimate use of allocators

One of the following scenarios usually calls for a custom allocator
* You've benchmarked, profiled and experimented your way to the conclusion that default STL memory manager `allocator<T>` is too slow, wastes memory or suffers excessive fragmentation for your STL needs, and you are certain you can do a better job yourself
* You found that `allocator<T>` takes precautions to be threadsafe, but you are interested only in single threaded execution and you don't want to pay for the synchronization overhead you don't need
* You know that objects in certain containers are typically useds together so you'd like to place them near one another in a special heap to maximize locality of reference
* You'd like to set up a unique heap that corresponds to shared memory, and put containers in that memory so they can be shared by other processes

E.g. suppose you have special routines modeled after `malloc` and `free` for managing a heap of shared memory
```
void* mallocShared(size_t bytesNeeded);
void freeShared(void *ptr);
```
And you'd like to put the content of STL containers via these in that shared memory:
```
template <typename T>
class SharedMemoryAllocator {
  public:
    pointer allocate(size_type numObjects, const void* localityHint = 0) {
        return static_cast<pointer>(mallocShared(numObjects * sizeof(T)));
    }

    void deallocate(pointer ptrToMemory, size_type numObjects) {
        freeShared(ptrToMemory);
    }
};

// you can then use this allocator like
using SharedDoubleVec = vector<double, SharedMemoryAllocator<double> >;

SharedDoubleVec v; // the elements in v will be allocated using
                   // SharedMemoryAllocator, but v itself, including all its
                   // data members will not be placed in shared memory.
                   // v is just a normal stack-based object.

// to put v's content and v itself onto shared memory, you have to do

void* pVectorMemory = mallocShared(sizeof(SharedDoubleVec)); // allocate memory
SharedDoubleVec *pv = new (pVectorMemory) SharedDoubleVec;   // placement new

... // use the object

pv->~SharedDoubleVec();    // explicit dtor call
freeShared(pVectorMemory); // deallocate memory

// don't bother with putting the container v itself onto shared memory if you
// don't need to.
```

Note that production code should handle `mallocShared` returning null pointer.

Another example. Suppose you have
```
class Heap1 {
  public:
    ...
    static void* alloc(size_t numBytes, const void* memoryBlockToBeNear);
    static void dealloc(void *ptr);
    ...
};

class Heap2 { ... }; // same interface

// suppose you'd like to colocate the contents of some STL containers in
// different heaps

template <typename T, typename Heap>
SpecificHeapAllocator {
  public:
    pointer allocate(size_type numObjects, const void* localityHint = 0) {
        return static_cast<pointer>(
            Heap::alloc(numObjects * sizeof(T)), localityHint);
    }
    void deallocate(pointer ptrToMemory, size_type numObjects) {
        Heap::dealloc(ptrToMemory);
    }
};

// then you use SpecialHeapAllocator to cluster containers' elements together
vector<int, SpecificHeapAllocator<int, Heap1> > v;
set<int, SpecificHeapAllocator<int, Heap1> > s;
// both v's and s's elements in Heap1

vector<int, SpecificHeapAllocator<int, Heap2> > v1;
set<int, SpecificHeapAllocator<int, Heap2> > s1;
// both v1's and s1's elements in Heap2
```

Note how `Heap1` and `Heap2` has to be types not objects, since otherwise we'd violate STL's constraint that different allocator objects of the same type should compare equal.

**Takeaways**
* custom STL allocators are useful in a number of contexts listed at the start: e.g. if std is proven slow and you can do better, not paying threadsafety cost, allocating to control locality, and allocating on particular segments of memory say shared memory mapped in multiple processes.
* remember to obey the constraint that all allocators of the same type must be equivalent.
