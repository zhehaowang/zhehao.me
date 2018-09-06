# The Concurrency API

C++11 introduced concurrency into the language and library, allowing platform-independent concurrent C++ code.

### Prefer task-based programming to thread-based

If you want to run a function doAsyncWork asynchronously, you can do
```cpp
int doAsyncWork();

std::thread t(doAsyncWork);          // thread-based

// or

auto fut = std::async(doAsyncWork);  // task-based. fut for future
// function object passed to std::async is considered a task
```

The task-based approach is superior: the caller of doAsyncWork likely is interested in its return value, with the task-based approach, the future object offers a get call to retrieve the return value, and if doAsyncWork throws, the future object can get access to that, too.
If the thread-based approach throws, the program dies via a call to std::terminate.

A more fundamental difference lies in the abstraction the task-based approach embodies: it frees you from thread management.

Three meanings of thread:
* Hardware thread, the one actually performing computation. Contemporary machine architectures offer one or more hardware threads per CPU core.
* Software (OS / system) threads. The threads OS manages across all processes and schedules for execution on hardware threads. You can have more OS threads than hardware threads. When one is blocked OS can switch to the execution of other unblocked ones.
* std::threads are objects in a C++ process that act as handles to underlying software thread. Some std::thread objects represent null handles i.e. no corresponding software threads, because they are default ctor'ed, have been moved from, have been joined (the function they were to run has finished), or have been detached.

Software threads are a limited resource. If you try to create more than the system can provide, a std::system\_error is thrown.

```cpp
int doAsyncWork() noexcept;          // see Item 14 for noexcept
// could throw:
std::thread t(doAsyncWork);          // throws if no more
                                     // threads are available
```

Well-written software must deal with the possibility of std::thread throw, but there is no obvious way. (run on current thread: potential unbalanced load; after some thread finishes: they could be waiting on something to be produced by doAsyncWork)

Even if you don't run out of threads, you could run into oversubscription: having more unblocked software threads than hardware threads, in which case CPU time slices software threads on the hardware, and performs context switches in between.
Cross-core switches are particularly expensive (CPU caches are cold for the new thread, whose execution will pollute the cache for the threads previously there).

Avoid oversubscription is difficult, since the dynamics of the number of unblocked software threads is hard to predict.

Your life will be easier if you dump these problems on somebody else, and in this case, std::async.

Using
```cpp
auto fut = std::async(doAsyncWork);   // onus of thread mgmt is
                                      // on implementer of
                                      // the Standard Library
```
Your likelihood of out-of-threads exception is greatly reduced, since std::async when called in this form doesn't guarantee a new thread will be created, rather it permits the scheduler to arrange the function doAsyncWork to be run on the thread requesting doAsyncWork's result (the thread calling get / wait on the future object). Reasonable schedulers take advantage of this freedom if the system is oversubscribed or is out of threads.

You could roll your own "run it on the thread needing the result", but std::async and the runtime scheduler (across the OS) is likely to have a better picture of the machine than you do.

However with just std::async the scheduler doesn't know which of your threads has tight responsiveness requirements (e.g. GUI), in which case you'll want to pass std::launch::async launch policy to the std::async (item 36)

State-of-the-art thread schedulers employ system-wide thread pools to avoid oversubscription, and they improve load balancing across hardware cores through work-stealing algorithms.
C++ Standards does not require the use of thread pools or work stealing, but some vendors implement this in their standard library and if you use a task based approach you reap the benefits automatically.

That said, there are scenarios where using threads directly may be appropriate. E.g.
* You need access to the API of the underlying threading implementation. (E.g. the underlying pthread or Windows thread may have richer set of features than C++ std::thread, in which case you can use native\_handle member function of std::thread which you can't do with std::future)
* You need to and are able to optimize thread usage for your application. (You know the execution profile, and are deploying it as the only significant process on a machine with fixed characteristics)
* You need to implement threading technology beyond the C++ concurrency API, e.g., thread pools on platforms where your C++ implementations don’t offer them.

These cases are uncommon. Most of the time, prefer task-based designs over thread-based.

**Takeaways**
* The std::thread API offers no direct way to get return values from asynchronously run functions, and if those functions throw, the program is terminated.
* Thread-based programming calls for manual management of thread exhaustion, oversubscription, load balancing, and adaptation to new platforms.
* Task-based programming via std::async with the default launch policy handles most of these issues for you.

### Specify std::launch::async if asynchronicity is essential

