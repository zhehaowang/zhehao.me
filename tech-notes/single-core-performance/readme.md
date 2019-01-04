# Writing (single core, single threaded) performant code

### The "what every programmer should know"s

* [Latency numbers](https://people.eecs.berkeley.edu/~rcs/research/interactive_latency.html)
  * Augmentations
* [Memory](https://people.freebsd.org/~lstewart/articles/cpumemory.pdf)
* 

### Memory and CPU cache

* Stack and heap
* Cache locality
* Helping branch prediction and instruction cache
* Math operations
* Lock free data structure

### IO and IPC

* IO multiplexing, `select`, `poll`, `KQueue`, `epoll`
* NIC user queue, DPDK, DMP
* IPC, shared memory, shared queue

### C++ language internals

* Choice of containers
* Object layout and CPU cache locality
* Allocators, custom new and delete
* Object pool
* Alignment
* Copy and move
* `inline`, function call cost, code bloat
* Pass by value / pointer / reference
* `template`, code bloat
* `noexcept`
* `constexpr`, `const` member function
* `virtual` and dynamic binding
* Smart pointers

### Profiling


### Particular subsystems

* Buffer (circular, etc)
* Socket abstractions
* Logging
* Interfacing shared memory
