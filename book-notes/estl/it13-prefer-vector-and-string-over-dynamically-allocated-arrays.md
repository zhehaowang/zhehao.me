# Prefer `vector` and `string` over dynamically allocated arrays

When you use a dynamic allocation, you'll need to make sure `delete` happens exactly once, and right form of `delete` / `delete[]` happens.

Any time you find yourself getting ready to dynamically allocate an array (writing `new T[...]`), you should consider using `vector` or `string` instead. (in general, use `string` when `T` is a character otherwise use `vector`, but there might be cases where `vector<char>` is useful which we'll go through later.)

When a `vector` or `string` is destroyed, its dtor automatically destroys the elements in the container and deallocates the memory holding those elements.

In addition, `vector` and `string` allows you to use the full arsenal of STL algorithms.

Integration with legacy code that uses dynamic arrays is generally not a problem.

The author can only think of legitimate concern in replacing dynamically allocated arrays with vectors or strings: many string implementations use reference counting behind the scene as an optimization to get rid of copies, but alas one programmer's optimization is another's pessimization.
If you use reference counting strings in a multithreaded environment, you may find the time saved by avoiding allocation and copying is dwarfed by the time spent on behind-the-scene concurrency control. 

Documentation would be best to figure out if your `string` impl uses reference counting. `string` --> `basic_string<char>` (and `wstring` --> `basic_string<wchar_t>`), so you really want to check `basic_string` template, its copy constructor.

If it is referenced counted and in a multi-threaded environment you decide that's a problem, then you can check if it's possible to make your library disable reference counting, or find an alternative that does not use reference counting, or use `vector<char>` instead of `string`.

**Takeaways**
* if you are dynamically allocating arrays, you are probably taking on more work than you need to. Use `vector` or `string` instead.
