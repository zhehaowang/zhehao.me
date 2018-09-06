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

When you std::async something, you are asking it to launch with a launch policy.
There are two defined:
* std::launch::async, means the function has to be run asynchronously (on a different thread)
* std::launch::deferred, means the function may run when get or wait is called on the future returned by std::async. When get or wait is invoked, the function will execute synchronously. If neither is invoked the function will never run.

The default launch policy is neither of the two but the or of them:
```cpp
// these two are the same thing
auto fut1 = std::async(f);                     // run f using
                                               // default launch
                                               // policy

auto fut2 = std::async(std::launch::async |    // run f either
                       std::launch::deferred,  // async or
                       f);                     // deferred
```

Using the default launch policy means if we do this on a thread t
```cpp
auto fut = std::async(f);   // run f using default launch policy
```
It's not possible to predict whether f will run concurrently with t, it's not possible to predict whether f runs on a thread different from the thread invoking get or wait on fut. It may not be possible to predict whether f runs at all.

The default launch policy mixes poorly with the use of thread\_local variables (TLS, thread local storage), because if f reads or writes such, it's not possible to predict which thread's variables will be acccessed.
```cpp
auto fut = std::async(f);        // TLS for f possibly for
                                 // independent thread, but
                                 // possibly for thread
                                 // invoking get or wait on fut
```
It also means loops like this one may run forever
```cpp
using namespace std::literals;        // for C++14 duration
                                      // suffixes; see Item 34

void f()                              // f sleeps for 1 second,
{                                     // then returns
  std::this_thread::sleep_for(1s);
}

auto fut = std::async(f);             // run f asynchronously
                                      // (conceptually)

while (fut.wait_for(100ms) !=         // loop until f has
       std::future_status::ready)     // finished running...
{                                     // which may never happen!
  …
}
```
In this case if f runs concurrently with the thread calling std::async (launch policy std::launch::async), there is no problem.
But if f is deferred, the status will never be ready (wait\_for and wait\_until on a task that's deferred yields the value std::future\_status::deferred).

This kind of bug can be hard to catch in unit testing: the scheduler may defer f only if the system is under heavy loads or threatened by oversubscription.

To fix this, we shouldn't enter the loop if the execution of f is deferred.
Like this:
```cpp
auto fut = std::async(f);                  // as above

if (fut.wait_for(0s) ==                    // if task is
    std::future_status::deferred)          // deferred...
{
                        // ...use wait or get on fut
  …                     // to call f synchronously


} else {                // task isn't deferred
  while (fut.wait_for(100ms) !=            // infinite loop not
         std::future_status::ready) {      // possible (assuming
                                           // f finishes)

    …                  // task is neither deferred nor ready,
                       // so do concurrent work until it's ready
  }

  …                    // fut is ready

}
```
Unfortunately there is no API on future object to tell if it's deferred, so we do a wait for 0s instead.

The upshot of these various considerations is that using std::async with the default policy for a task is fine as long as
* the task need not run concurrently with thread calling get or wait
* it doesn't matter which thread's local storage are read or written
* either there's guarantee that get or wait will be called on the future returned by std::async, or it's acceptable that the task may never execute
* code using wait\_for and wait\_until takes the possibility of deferred status into account

If any of the conditions do not hold, you may want to guarantee a truly asynchronous execution:
```cpp
auto fut = std::async(std::launch::async, f);  // launch f
                                               // asynchronously
// or have this generic tool
// C++14
template<typename F, typename... Ts>
inline
auto
reallyAsync(F&& f, Ts&&... params)       // return future
{                                        // for asynchronous
  return std::async(std::launch::async,  // call to f(params...)
                    std::forward<F>(f),
                    std::forward<Ts>(params)...);
}

// use reallyAsync just like async
auto fut = reallyAsync(f);         // run f asynchronously;
                                   // throw if std::async
                                   // would throw

// C++11
template<typename F, typename... Ts>
inline
std::future<typename std::result_of<F(Ts...)>::type>
reallyAsync(F&& f, Ts&&... params)       // return future
{                                        // for asynchronous
  return std::async(std::launch::async,  // call to f(params...)
                    std::forward<F>(f),
                    std::forward<Ts>(params)...);
}
```

**Takeaways**
* The default launch policy for std::async permits both asynchronous and synchronous task execution.
* This flexibility leads to uncertainty when accessing thread\_locals, implies that the task may never execute, and affects program logic for timeout-based wait calls.
* Specify std::launch::async if asynchronous task execution is essential.

### Make std::threads unjoinable on all paths


