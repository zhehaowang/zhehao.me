# Different flavors of caching

### Processor architecture

##### Read policy

* Look through: client queries cache first, if cache misses, query backing storage next
* Look aside: client queries cache and backing storage at the same time

(https://www.sqa.org.uk/e-learning/CompArch02CD/page_29.htm)

##### Write policy

* Write through: write is done synchronously to cache and backing storage, simple to implement but worse performance
* Write back (write behind): initially write is done to cache, and write to backing storage is postponed until the modified content is about to be replaced by another cache block (laziness; write back to backing storage only when you have to do so)

(https://en.wikipedia.org/wiki/Cache_(computing))

### Distributed cache

