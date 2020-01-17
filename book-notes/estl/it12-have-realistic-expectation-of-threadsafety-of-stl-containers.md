### Have realistic expectations of thread-safety of STL containers

STL threadsafety may vary from implementation to implementation.

In essence, the most you can hope for from an implementation is the following:
* multiple readers are safe. multiple threads can simultaneously read the contents of a single container. There must not be any writers acting on the container during the reads.
* multiple writers to different containers are safe.

This is what you can hope for, not what you can expect. Some STL implementation don't offer such.

Making containers thread-safe out of the box is hard.
Consider the following ways:
* lock a container for the duration of each call to its member functions
* lock a container for the lifetime of each iterator it returns (e.g. via `begin()`, `end()`)
* lock a container for the duration of each algorithm invoked on that container. (this actually makes no sense as algorithms have no way to identify the container on which they are operating.)

Now consider the following code:
```
vector<int> v;
vector<int>::iterator first5(find(v.begin(), v.end(), 5)); // l1
if (first5 != v.end()) {                                   // l2
    *first5 = 0;                                           // l3
}
```
In a multithreaded environment, a different thread can modify the data in `v` right after completion of `l1`, and `l2`'s check would be meaningless.
Similarly, the assignment on `l3` is unsafe.

None of the proposals above would prevent these problems.
For this code to be threadsafe, it must be locked from `l1` through `l3`, and it's difficult to imagine how STL implementation could deduce that automatically.

Bearing in mind the typically high cost of synchronization primitives it's even more difficult to imagine how an implementation could do it without imposing significant performance penalty.

In this case, you'll have to manually take charge of synchronization control. E.g. using an `RAII` lockguard, or one that is templated on the container type.
```
template <typename Container>
class Lock {
  public:
    Lock(const Container& container) : c(container) { getMutexFor(c); }
    ~Lock() { releaseMutexFor(c); }
  private:
    const Container& c;
};
// details have been omitted

vector<int> v;
...
{
    Lock<vector<int> > lock(v);
    vector<int>::iterator first5(find(v.begin(), v.end(), 5));
    if (first5 != v.end()) {
        *first5 = 0;
    }
}
```

**Takeaways**
* You can hope for an STL implementation that allows multiple threads to read from one container and multiple writers on separate containers.
* You can't rely on any thread support at all.
