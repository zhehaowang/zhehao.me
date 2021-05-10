### [glibc](https://sourceware.org/glibc/wiki/MallocInternals)



Also [reference](https://reverseengineering.stackexchange.com/questions/15033/how-does-glibc-malloc-work)

### [tcmalloc](https://google.github.io/tcmalloc/design.html) (Thread Caching malloc)

vs glibc.
* Parallel application: most allocation/deallocation does not take locks
* Diverging from standard where performance benefits warrant
* Extension to allow metric gathering (low overhead sampling)

Does not throw an exception when allocations fail, but instead crashes directly, unlike in the standard implementations.

* Frontend cache
  * application facing, has a cache of memory for allocation / hold free memory.
  * defaults to per-logical-CPU cache now as opposed to per-thread cache, so does not require any locks.
  * satisfy request if has cached memory of particular size, if not, request a batch from middle end.
  * if middle-end exhausted or requested size larger than max size frontend caches, request goes to backend directly to either get satisfied directly or refill middle end.
  * Allocation of small objects are rounded up to the nearest size class (e.g. 12B getting rounded up to 16B).
  * When deallocating, if small, put back into frontend cache, otherwise returned to backend (pageheap).
* Middle-end
  * acquire lock to access.
  * Transfer cache. Frontend reaches out to transfer cache for and to return memory (holds array of pointers to free memory, transfer in that different threads talk to this area to free/grab memory)
  * Central free list. Manages memory in spans (collection of tcmalloc pages of memory).
* Backend (pageheap): fetching memory from OS.
  * Freelist of collections of 1, 2, ... 255 tcmalloc pages.

Core idea seems a per-logical-CPU per-size cache of memory that is lockfree to access + freelists of pages of the same size + metric gathering (sampling).
