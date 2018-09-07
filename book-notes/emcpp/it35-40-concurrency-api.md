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

Every std::thread is in one of two states, joinable or unjoinable.

A thread corresponding to an underlying thread that's blocked or waiting to be scheduled is joinable. std::thread objects corresponding to underlying threads that have completed are also considered joinable.

Unjoinable thread objects include
* default ctor'ed std::thread, no function to execute
* std::threads that have been moved from, whose underlying execution now corresponds to a different thread
* std::threads that have been joined. After the join the thread object no longer corresponds to the underlying thread of execution that has finished running
* std::threads that have been detached. detach severs the connection between a std::thread object and the underlying thread of execution it corresponds to.

We care about the joinability of a thread, because calling dtor on a joinable thread will cause the entire process to be terminated.

Say we have the following (we use the thread-based approach as opposed to task-based since we want to configure the priority of this thread, the native handle part)
```cpp
constexpr auto tenMillion = 10'000'000;       // see Item 15
                                              // for constexpr

bool doWork(std::function<bool(int)> filter,  // returns whether
            int maxVal = tenMillion)          // computation was
{                                             // performed; see
                                              // Item 5 for
                                              // std::function

  std::vector<int> goodVals;                  // values that
                                              // satisfy filter

  std::thread t([&filter, maxVal, &goodVals]  // populate
                {                             // goodVals
                  for (auto i = 0; i <= maxVal; ++i)
                   { if (filter(i)) goodVals.push_back(i); }
                });

  auto nh = t.native_handle();                // use t's native
  …                                           // handle to set
                                              // t's priority
  if (conditionsAreSatisfied()) {
    t.join();                                 // let t finish
    performComputation(goodVals);
    return true;                              // computation was
  }                                           // performed

  return false;                               // computation was
}                                             // not performed
```
If conditionsAreSatisfied returns true, this is all good; if it returns false or throws an exception, as the stack unwinds dtor will be called on a joinable t, and the process would halt.

Why does a std::thread dtor behave this way? Because the other options are worse.
An implicit join means dtor would wait for the asynchronous execution to finish. This is counter-intuitive to debug.
An implicit detach, the underlying thread would continue to run but its connection to the thread object is severed.
In the above code example where goodVals local variable is passed by reference to the thread function, when detach happens, doWork finishes and goodVals unwinds, the running thread would be looking at a stack frame that's popped (or worse, occupied by a later function call).

This puts the onus on you to ensure if you use a std::thread object, it's made joinable on every path out of the scope in which it is defined.
Any time you want to such things as making sure to perform some action along every path out of a block, RAII naturally comes in mind.
RAII is predominant in the Standard Library, like std::unique\_ptr std::weak\_ptr, std::shared\_ptr and std::fstream dtors, etc.

The Standard Library does not have an RAII class for std::thread, you could do one such yourself that takes in a DtorAction:
```cpp
class ThreadRAII {
public:
  enum class DtorAction { join, detach };    // see Item 10 for
                                             // enum class info

  ThreadRAII(std::thread&& t, DtorAction a)  // in dtor, take
  : action(a), t(std::move(t)) {}            // action a on t
  // in ctor, note that we only move thread to be managed by this
  // RAII object, note that we can't copy std::thread objects
  
  ~ThreadRAII()
  {
    if (t.joinable()) {
      // joinability test is necessary since join or detach on
      // unjoinable threads yields UB.
      if (action == DtorAction::join) {
        t.join();
        // If you are worried about the potential race condition
        // between joinable check and join / detach actions, such
        // worries are unfounded: a std::thread object can change
        // state from joinable to unjoinable only through a member
        // function call or a move operation. At the time a ThreadRAII
        // object's dtor'ed invoked, no other thread should be making
        // member function calls on that object.
        // The client code still could invoke dtor and something else
        // on the object at the same time, but should be made aware
        // such calls could result in race conditions.
      } else {
        t.detach();
      }
      
    }
  }

  std::thread& get() { return t; }           // see below

private:
  DtorAction action;
  std::thread t;
  // note the order of data members. In this case it doesn't matter
  // but you usually want to put them to last in a class's members
  // since once initialized they may start running immediately, and
  // running them may require other member variables to be already
  // initialized.
};
```

And our client code now looks like
```cpp
bool doWork(std::function<bool(int)> filter,  // as before
            int maxVal = tenMillion)
{
  std::vector<int> goodVals;                  // as before

  ThreadRAII t(                               // use RAII object
    std::thread([&filter, maxVal, &goodVals]
                {                             
                  for (auto i = 0; i <= maxVal; ++i)
                    { if (filter(i)) goodVals.push_back(i); }
                }),
                ThreadRAII::DtorAction::join  // RAII action
  );

  auto nh = t.get().native_handle();
  …

  if (conditionsAreSatisfied()) {
    t.get().join();
    performComputation(goodVals);
    return true;
  }

  return false;
}
```

Note that join is still not a desired behavior in that it could lead to performance anomaly, or even hung a program (item 39). The proper solution would be to communicate to the asynchronously running lambda that we no longer need its work and it should return early.
But there is no such support for interruptible threads in C++. They can be implemented by hand but is beyond the scope of now.

Since we've custom dtors compiler will suppress move operations generation, there's no reason ThreadRAII's not movable, so we could add
```cpp
  ThreadRAII(ThreadRAII&&) = default;               // support
  ThreadRAII& operator=(ThreadRAII&&) = default;    // moving
```

**Takeaways**
* Make std::threads unjoinable on all paths (e.g. through RAII).
* join-on-destruction can lead to difficult-to-debug performance anomalies.
* detach-on-destruction can lead to difficult-to-debug undefined behavior.
* Declare std::thread objects last in lists of data members.

### Be aware of varying thread handle destructor behavior

A joinable std::thread corresponds to an underlying system thread of execution, a future for a non-deferred task has a similar relationship to a system thread.
As such, both std::thread objects and future objects can be thought of as handles to system threads.

Yet, dtor on a joinable thread results in program termination, but dtor on a future sometimes behaves as if it did an implicit join, sometimes detach, and sometimes neither. It never causes program termination.

Think about the execution of a future object, the asynchronously executed callee needs to transmit result back to the caller (typically via a std::promise object), but where does the result live?
The callee could finish before caller invokes get.
It cannot live in the std::promise as that object, being local to the callee, would be destroyed when the callee finishes.
The result cannot live in the caller's future, either, because a std::future may be used to create a std::shared\_future (thus transferring ownership of the caller's result from the std::future to the std::shared\_future), which may then be copied many times after the original future is destroyed.
Given that not all result types can be copied (move-only, for example), and the result must live as long as the last future referencing to it, which of the potentially many futures corresponding to the callee should be the one to contain its result?

Since neither caller or callee are suitable for storing the callee's result, it's stored in a location outside both known as the shared state. Its implementation is typically heap based but not specified by the standards.
The dataflow is illustrated as such
```
Caller <-- future -- Shared State (Callee's Result) <-- std::promise (typically) -- Callee
```

The behavior of the dtor of a future is determined by the shared state associated with it.
* The dtor for the last future referring to a shared state for a non-deferred task launched via std::async blocks until the task completes. (implicit join)
* The dtor for all other futures simply destroys the future object. (for asynchronously running tasks, this is an implicit detach; for deferred tasks for which this is the final future, it means the deferred task will never run)

In other words, the dtor of a future is a normal behavior and one exception
* The normal behavior is that a future's dtor destroys the future object. It doesn’t join with anything, it doesn’t detach from anything, it doesn’t run anything. It just destroys the future’s data members. (and decrements the reference count inside the shared state that's manipulated by both the futures referring to it and the callee's std::promise)
* The exception happens when the future 1) refers to a shared state that was created due to a call to std::async, 2) the task's launch policy is std::launch::async (can be explicitly specified or decided by the runtime system), and 3) the future is the last future referring to the shared state (this matters for shared\_futures). This exceptional behavior is to implicitly join.

This decision of implicit join special case is controversial, but present in both C++11 and C++14.

Future's API offers no way to determine whether a future refers to a shared state arising from a call to std::async, so given an arbitrary future object, it's not possible to know whether it'll block on dtor or not. E.g.
```cpp
// this container might block in its dtor, because one or more
// contained futures could refer to a shared state for a non-
// deferred task launched via std::async
std::vector<std::future<void>> futs;   // see Item 39 for info
                                       // on std::future<void>

class Widget {                         // Widget objects might
public:                                // block in their dtors
  …

private:
  std::shared_future<double> fut;
};
```
Unless you know in your program logic for all these futures one of the three conditions won't be met. Like this
```cpp
int calcValue();                      // func to run

std::packaged_task<int()>             // wrap calcValue so it
  pt(calcValue);                      // can run asynchronously

auto fut = pt.get_future();           // get future for pt
// this future does not refer to a shared state created by
// std::async, and its dtor should not block

// To illustrate why have the special case for reference to
// shared states arose due to a std::async:
// say instead of std::async, you create a thread on the future
std::thread t(std::move(pt));
// packaged_task cannot be copied

... // end block

// if nothing happens to t before end block, t will be joinable
// and program will terminate
// if a join is done on t, there is no need for fut to block its
// dtor because join is already present
// if a detach is done on t, there is no need for fut to detach
// in its dtor because the calling code already does that
```

**Takeaways**
* Future dtors normally just destroy the future’s data members.
* The final future referring to a shared state for a non-deferred task launched via std::async blocks until the task completes.
