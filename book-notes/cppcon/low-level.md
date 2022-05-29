# Assembly

[Godbolt](https://www.youtube.com/watch?v=bSkpMdDe4g4)
[Slides](https://github.com/CppCon/CppCon2017/blob/master/Keynotes/What%20Has%20My%20Compiler%20Done%20for%20Me%20Lately%20-%20Unbolting%20the%20Compiler%27s%20Lid/What%20Has%20My%20Compiler%20Done%20for%20Me%20Lately%20-%20Unbolting%20the%20Compiler%27s%20Lid%20-%20Matt%20Godbolt%20-%20CppCon%202017.pdf)

### Basics

##### Registers
rax (64b), eax (32b, higher 32b zeroed on write), ax (16b), ah (8b) al (8b)

* Register / ABI
rax - retval
rdi, rsi, rdx, rcx - params
rbp
rip
r8-r15
xmm0-xmm15

##### Intel syntax

```
op
op dest
op dest src1
op dest src1 src2
```

Examples
```
mov eax, dword ptr [r14]          ;  int eax = *14;
add rax, rdi                      ;  rax += rdi;
add eax, dword ptr [r14 + 4*rbx]  ;  eax += r14[rbx];
lea eax, [r14 + 4 * rbx]          ;  int *eax = &r14[rbx];
xor eax, eax                      ;  eax = 0;
```
Note double word is 16b.

Note the following common practices:
* Using `lea` for computation, instead of an `add` for the following, you may see
```
lea rax, [r14 + 4 * rbx]
```
Reason could have to do with you put the result directly into a designated place, maybe e.g. for retval
* Using `xor eax, eax` for ret 0: reason could have to do with shorter instruction than a `mov eax, 0`
* Avoiding expensive `div`
```
unsigned divide(unsigned x) { return x / 3; }

mov eax, edi         ;  eax = x
mov edx, 0xaaaaaaab  ;  edx = 0xaaaaaaab (2/3 shifted up by 2^32)
mul edx              ;  eax:edx = eax * edx (gets answer * 2^32)
mov eax, edx         ;  discards bottom 32 bits (gets answer * 2)
shr eax              ;  eax /= 2 (gets answer. why 2/3 and not 1/3? to cover the range of unsigned)
ret
```
And a modolus % 3 is above plus the following
```
lea eax, [rdx + rdx * 2]  ;  eax = result of division x3
sub edi, eax              ;  edi -= eax
mov eax, edi              ;  return eax
ret
```

How godbolt annotates c++ line / asm line correspondence: compiler generates this info with `-g` flag.

Google benchmark online wrapper https://quick-bench.com/

### Globals, linker

[talk](https://www.youtube.com/watch?v=xVT1y0xWgww)

Globals common uses: Logging, intrusive profiling, polymorphic factories

Code sample
```cpp
// static.h
extern std::string g_str;

// static.cc
std::string g_str = "something too long for SSO";

// dynamic.h
#include "static.h"

std::string& getGlobal();

// dynamic.cc
std::string& getGlobal() { return g_str; }

// main.cc
#include "dynamic.h"
#include "static.h"

int main() {
    std::cout << g_str << "\n";
    std::cout << getGlobal() << "\n";
}
```

If we link the above program in particular ways, it'll crash on double free: compile and archive the `static` library, compile and link statically against `static` to produce `dynamic`, and when building `main`, link against `static` first and then `dynamic`.

The printed `g_str` will live in main's address space (because `static` is link i.e. copied in first), its ctor will get called at entrace.
Because `dynamic` also statically linked in `static`, it'll also have a `g_str`, whose ctor will also get called when the shared library gets initialized at entrance, at the same address of main's `g_str` (linker thought them one and the same).
Then `atexit`, `static` and `dynamic` for `g_str` in main's address space will both happen and lead to a crash.

Since C++17, `inline std::string g_str` in the `.cc` solves it.

Ways to deal with undefined order of global instantiation in different TUs.

Consider destructor dependency: whereas appropriate, there might be a case for the likes of `folly::Indestructible` (uses a `union` with just template member `T` to avoid `~T` getting called).

What linker does:
- looks at link line left to right
- each target / library provides some symbols (`T`) and need others (`U`). `W` for weak symbols where multiple occurrences are Ok.
- pick up needed symbol at each encountered target / library
- each time a provided symbol matches one that's needed
  - if static, copy and paste assembly into target
  - if dynamic, say the loader will get this

[See also](tech-notes/linker/readme.md)

Different from traditional unix linker, clang linker (lld) keeps track of defined symbols even if they aren't requested by an object already seen, with this you don't have to specify the same library multiple times.

# What else has my compiler done for me lately

[Talk](https://www.youtube.com/watch?v=nAbCKa0FzjQ)
[Slides]()

### alias'ing / __restrict

Consider this code
```c++
void maxArray(double* __restrict x, double* y) {
    for (int i = 0; i < 65536; ++i) {
        if (y[i] > x[i]) {  // same effect as std::max
            x[i] = y[i];
        }
    }
}
```

Without the `restrict` keyword, i.e. the two input memory areas won't overlap, compiler will first check to see if they overlap, and if they don't, use vector instructions to make this faster.
With restrict, we skip those checks.

Pointer aliasing: one location in memory can be referred to by more than one name (e.g. with different pointers). Restrict effectively says alias'ing won't happen.

### Devirtualization

Instead of a virtual call, do a switch case comparison and possibly inline the function to call.

### fbstring, gcc strings

[talk](https://www.youtube.com/watch?v=kPR8h4-qZdk)

##### pre-`gcc5` `std::string`

`<gcc5`, `string` is implemented as 8B pointer into start of a string, prefixed with size, capacity and refcount.

```
                   +----------+
                   |   size   | 
                   +----------+
                   | capacity | 
                   +----------+
                   | refcount | 
+------------+     +----------+
| string obj | --> |   data   |
+------------+     +----------+
```

Empty string is special: it is implemented as a pointer to global var with just `\0`. (you don't want to heap allocate 1 byte and prefix just for all the default constructed empty strings.)

why not just have `nullptr` be empty string? we don't want branching in all the `size` calls etc.

`<gcc5` also does CoW, no longer allowed since c++11 for concurrency reasons.

we store `refcount - 1`, so that default state zero loaded at boot time, empty string does not require any additional processing.

##### `fbstring`

`fbstring` 24B string with SSO.
How much size do we have? Norman answer: 22. `\0` 1B and `size + union` 1B. But if we store instead remaining size on stack, then `0` double duties as `\0` and size, and we get 23.
```
+----------+
|   size   | 
+----------+
| capacity |
+----------+
| data_ptr |
+----------+

(SSO)
+---------------------+
|                     | 
|      data_23B       |
|                     |
+---------------------+
| remaining_size_byte |
+---------------------+
```

What about union bit?

jemalloc fb: rounds up 29B to 32B, and return one bucket of 32B size. `fbstring` knows to work with jemalloc to see if there are actual additional room behind what's returned, and use the space there for union bit. (clang instead uses one bit from capacity: no string is allowed more than `2^63`)

fbstring also has large size optimization: when larger than 255B, use CoW.

fbstring does result in branching (and longer / more complicated asm) when trying to get `size` while gcc version doesn't, but benchmark with a large set of strings suggests the fb one can be faster because of memory access cost of gcc: gcc can have much higher l1cache misses compared with fbstring.

##### post-`gcc5` `std::string`

`>gcc5` new string layout, dropped CoW because of std, and adopted SSO by default. The new layout is 32B data ptr, size, capacity, on-stack data.
If on stack, data ptr points to the start of capacity instead, and we get 15B + nullptr as maximum string size we can hold on stack.

```
+------------+     +------------+
|  data_ptr  | --> |    data    |
+------------+     +------------+
|    size    |
+------------+
|  capacity  |
+------------+
| stack data |
+------------+

(SSO)
+------------+
|  data_ptr  | --+
+------------+   |
|    size    |   |
+------------+ <-+
|            |
| stack data |
|     /0     |
+------------+
```

`data` and `size` need no branching with this layout.

### Type Deduction and Why You Care

[Talk](https://www.youtube.com/watch?v=wQxj20X-tIU)

# (Multi / virtual) inheritance layout

[Reference](https://shaharmike.com/cpp/vtable-part1/)

```cpp
struct Mother {
  virtual void foo();

  uint64_t motherData;
};

struct Father {
  virtual void bar();

  uint64_t fatherData;
};

struct Child : public Mother, Father {
  void foo() override;
  void bar() override;

  uint64_t childData;
};
```

Clang
```
Object Child                 vtbl of Child
+---------------------+      +------------------------------------+
| Mother / Child vptr | ---> | top_offset (0)                     |
+---------------------+      +------------------------------------+     +--------------------+
|     Mother data     |      | type_info* for child               | --> | type_info methods* |
+---------------------+      +------------------------------------+     +--------------------+
|     Father vptr     | -+   |  Child::foo()*                     |     |  type name string  |
+---------------------+  |   +------------------------------------+     +--------------------+
|     Father data     |  |   |  Child::bar()*                     |     |  parent type_info* |
+---------------------+  |   +------------------------------------+     +--------------------+
|      Child data     |  +-> | top_offset (-16)                   |
+---------------------+      +------------------------------------+
                             | type info* for child               |
                             +------------------------------------+
                             | non-virtual thunk* to Child::bar() |
                             +------------------------------------+
```
Why this layout?
* The first part of the vtbl is like a single-inheritance, calling `Child::foo()` via a `Mother* c = new Child()` just works.
* We need to support calling `Child::bar()` via a `Father* c = new Child()`, when presented with that, the **non-virtual thunk** needs to offset `this` object to the start of `Child` object (using `top_offset`) and resolve the call to `Child::bar()`.

This is what gdb means when it's in some non-virtual thunk to call some function.

TBC: virtual inheritance

# Speed is found in the minds of people, sorting algorithms

[talk](https://www.youtube.com/watch?v=FJJTYQYB1JQ)

Quicksort. When few enough elements remain, use linear scan insertion sort.

What about binary search to find where the new element should go in insertion sort?
Fewer comparisons but slower than linear, because binary maximizes information with each comparison, and branch prediction is correct only ~50% of time, while branch prediction in linear case is much easier: always predict success and you will mispredict only once.

When you can get away with bit operation and avoid branching, do that.

Middle-out insertion sort (fewer moves), but time is identical compared with naive insertion sort.

We often try clever things that don't work.

First make heap, then insertion sort. Fewer swaps, fewer comparisons.
Can be optimized to be faster than std::sort.

A better proxy for time spent: number of compares, swaps and the distance between two consecutive array accesses.

# How we get to main

[talk](https://www.youtube.com/watch?v=dOfucXtyEsU)

`objdump -dC a.o` does not fill relocation of that object file, so if there is a call to some functions in a different object file, it'll be `opcode-call 00 00 00 00` which almost looks like calling the next instruction.

`objdump --reloc -dC a.o` interleaves with content of relocation section.
After interleaving we are basically given what linker would see, and the linker needs to patch the code to fill in with where those calls should actually go.

Even for calls in the same translation unit, the compiler might opt to put functions into different sections and let the linker fill in the actual address.

`objdump --syms -dC a.o` shows syms (defined and undefined), similar to `nm`.

linker reads all the inputs, identify all the symbols, applies relocations.

Linker script (`-Wl,--verbose`) what to do with sections

What's within a section is an opaque blob to the linker, meaning it can't discard unused functions in a section.
We could let the compiler do `--ffunction-sections,-fdata-sections` (putting each unique function and blob of data into their own sections), and let linker run with `-Wl,--gc-sections` to discard them. (this could affect optimization between functions calls?)

### dynamic linking

your executable e.g. doesn't have libstdc++ in it, and this is where dynamic linking comes in.

your calls to functions within a dynamic shared object will point to an address in Procedure Linkage Table (@plt).

The first time we make that call, PLT will not have the actual address for the function called and needs another lookup, but once we do have the address we'll fill it in such that subsequent calls will jump more cheaply (lazy).

Why do it lazily?
We don't know what functions from the library will get called. If we resolve them all upfront, we'll slow down the startup time.

`LD_BIND_NOW` should disable the lazy behavior: fill in PLT as opposed to asking resolver.

`ldd your_executable`, env var `LD_DEBUG=all|help` help debug what's loaded and how the lazy loading works.

This lazy behavior also allows `LD_PRELOAD`, where you can supply the needed symbols yourself (e.g. those in stdc library).

High performance code should probably avoid dynamic linking -- a barrier for optimization (PLT as one level of indirection, LTO can't see through).

### More

Weak references: c++ `inline` functions gets marked a weak symbol -- it's ok to have many of these symbols and you get to pick which one is the implementation.

LTO: compiler generates an intermediate form to tell the linker, and linker when linking and talk back to the compiler to see the code for something.

# Tuning C++: Benchmarks, and CPUs, and Compilers! Oh My!

[talk](https://www.youtube.com/watch?v=nXaxk27zwlk)

### First step to tuning, measure.

### Second step, understand the measurement.

Consider googlebenchmark'ing `vector<int>::push_back` code:
```cpp
// just this:
static void bench_pushback(benchmark::State& state) {
    while (state.keepRunning()) {
        vector<int> v;
        v.push_back(42);
    }
}
// not great, we might essentially be benchmarking malloc.

// the above + baseline:
static void bench_baseline(benchmark::State& state) {
    while (state.keepRunning()) {
        vector<int> v;
        (void) v;
    }
}
// not great, the baseline doesn't do malloc.

// reserve as a baseline to do malloc, so this
static void bench_baseline(benchmark::State& state) {
    while (state.keepRunning()) {
        vector<int> v;
        v.reserve(1);
    }
}
// vs test
static void bench_pushback(benchmark::State& state) {
    while (state.keepRunning()) {
        vector<int> v;
        v.reserve(1);
        v.push_back(42);
    }
}
// Not great, everything got magically faster -- optimizer realized it does nothing and deleted
// everything.
// Why was the optimizer able to delete after the addition of reserver?
// Handwavy: push_back has a conditional reserve. When analyzing push_back with reserve, the
// optimizer gets confused while when analyzing each separately optimizer realized it can delete
// things.
// Why not just turn down -O level? We want fine grained control: in general we still want optimized
// code.

// to avoid optimizer deleting everything, try
static void escape(void *p) {
    asm volatile("" : : "g"(p) : "memory");
    // volatile: this asm code has observable side effect (that may not be known to the compiler.
    // optimizer shouldn't try to optimize this asm code.
    // a good use case for this volatile might be, e.g. I want to produce exactly 8B of instructions
    // here because later on I might rewrite those as we execute (e.g. hot patching or jit).
    
    // the syntax goes as : outputs : inputs : clobbers (what about our program can change). so this
    // line takes our memory address, outputs nothing, and tells our compiler any memory area can be
    // read or written.
}

static void clobber() {
    asm volatile("" : : : "memory");
    // any memory area can be read or written.

    // why not forget about `escape` and just use `clobber`? `clobber` sees just memory, and with it,
    // compiler is still free to optimize if the vector never live in memory. But with `escape`, a
    // memory address is given so it forces something to live in memory, and in combination with
    // "memory", that void* cannot be optimized away. It's like if we never take the address of
    // something, it may not even live in memory.
}

// then a better version
static void bench_create(benchmark::State& state) {
    while (state.keepRunning()) {
        vector<int> v;
        // we need to keep the stack object around
        escape(&v);
        (void)v;
    }
}
// vs reserve
static void bench_reserve(benchmark::State& state) {
    while (state.keepRunning()) {
        vector<int> v;
        v.reserve(1);
        // we need to keep just the heap allocated data* around
        escape(v.data());
    }
}
// vs test
static void bench_pushback(benchmark::State& state) {
    while (state.keepRunning()) {
        vector<int> v;
        v.reserve(1);
        escape(v.data());
        v.push_back(42);
        // make sure push_back happens
        clobber();
    }
}
```

### `perf`

`perf stat`, it gives
* task-clock: CPU time (if you are switched off core, we don't count that time)
* seconds elapsed: wall time
Where we are stalled in the CPU pipeline
* stalled cycles frontend
* stalled cycles backend
* ...

`perf record`: sampling with periodic signal interrupt, `perf report`: just breakdown without being able to expand.

`perf record -g`, `perf report -g`: record and report call graph info (when interrupted, look at the current running thread, and try to figure out the call stack). Gives "+" with `enter` you can expand.
`perf report -g` call graph is kinda inversed: it shows each callee (every place that function's called), and when you expand you see callers to them.
`perf report -g 'graph,0.5,caller'`: `0.5` the filter, `caller` inverts the graph, for a view closer to flamegraph.

Caution:
You are likely going to see artifacts of `perf` measurement in `perf report` (similar to heisenberg uncertainty).
Similarly, it's not crazy for an expensive operation to have its cost attributed to, say, a memory write or jump right after it.

`a` in `perf report` annotates the function with assembly.

`-fno-omit-frame-pointer`: stop deleting the frame pointer. This register tells you where the bottom of the stack frame is (_is this not bp_), and you can then walk the stack. Compilers usually omit this to get 1 extra register.

`--benchmark_filter` option of `perf` works nicely with google benchmark.

# CppCon 2015: Bryce Adelstein-Lelbach “Benchmarking C++ Code"

[talk](https://www.youtube.com/watch?v=zWxSZcpeS8Q)

HPC performance benchmark.

Observational error in performance measurement in avoidable, and a statistical test is needed.
* random errors: natural variance
* systematic errors

Sources of variance, hardware jitter, OS, observer effect, etc.

Figure out dep, indep, control vars. Determine metrics, assume independence of variables, assume normal distribution.

Hot and cold effect: skip the first iterations.

Population standard deviation: `1/n` <-- use when you have the entire population
Sample standard deviation: `1/(n - 1)` <-- use when you have samples of the population

Confidence intervals, constructed from confidence level, data (including sample size), uncertainty of the data.

Given a margin of error, a critical value, an uncertainty and a mean, you can compute the right sample size needed (using an initial pilot of samples) -- if this requires a huge size then the experiment probably needs to be redesigned.

Mean - median test, normality test: if a dataset fits a normal distribution well.

### Time-based benchmarking

Monotonic clock -- frequency stable (turboboost etc), monotonic, but accessed at higher overhead (suitable for mics and up, not good for nanos scale), *nix use `CLOCK_MONOTONIC`.

`std::chrono`
* `system_clock`: wall clock, can go back
* `steady_clock`: monotonic clock
* `high_resolution_clock`: clock with the shortest tick period (often an alias of one of the previous 2, and platform dependent)

rdtsc -- monotonic, lower overhead to access (good for nanos).
* Resolution is in CPU cycles but not always frequency stable (newer architectures are frequency stable - constant TSC, older ones are not), ticks with base clock cycles (100MHz / 133MHz).
  * Constant TSC means means we are good at measuring time, but bad at measuring number of CPU cycles -- for the same number of constant TSC ticks, the CPU frequency might have changed underneath you.
  * Non-constant TSC is the reverse -- good at measuring CPU cycles, but bad at measuring time.

### Memory

goopleperftools tcmalloc, overloading operator new/delete, etc

Counting copies / moves: consider passing mock objects that does counting.

### Hardware performance counter

Low overhead and informative, but microarchitecture specific, some are estimates and need specialized knowledge to use these for performance analysis (e.g. Linux PAPI).

Consider Intel VTune Amplifier over Linux PAPI to get around some needing some specialized knowledge.
* Sampling based
* Gets data from timers, hardware performance counters, OS metrics
* Perf can be viewed in source / asm granularity
* Analyzes kernel calls, threads, subprocesses
* Support for instrumenting parallel and distributed code (which can use vtune API to tell profiler about threading, concurrency data structure, etc)
* Support for user defined analysis passes
* gui

### Writing performance tests

* Stateful, comparison vs previous
* Stateless, benchmark

# Want faster C++? Know your hardware

[talk](https://www.youtube.com/watch?v=BP6NxVxDQIs)

prefetching, cache associativity, false sharing, data dependency, denormal floats

# When a Microsecond Is an Eternity: High Performance Trading Systems in C++

[talk](https://www.youtube.com/watch?v=NH1Tta7purM)

Sometimes the optimizer can do some of these for you, and these instances could help it do the right thing.

* Slow path removal
```cpp
if (errorA) {
    handleA();
} else if (errorB) {
    handleB();
} else if (...) {
    ...
} else {
    sendOrder();
}

// vs
int64_t errFlags;
if (!errFlags) {
    sendOrder();
} else {
    // don't inline this
    handleError(errFlags);
}
```
* Template based configuration to replace dynamic dispatch
* Prefer template to branches in the hot path, but don't go crazy with this
```cpp
void runStrategy() {
    calcPrice(s, p, c);
    ...
}

double calcPrice(Side side, double px, double commission) {
    return side == Side::Buy ? px + commission : px - commission;
}

// vs
template<Side T>
void Strategy<T>::runStrategy() {
    calcPrice(s, p, c);
    ...
}

double Strategy<Side::Buy>::calcPrice(double px, double commission) {
    return px + commission;
}

double Strategy<Side::Sell>::calcPrice(double px, double commission) {
    return px - commission;
}
```
* Lambdas are fast and convenient
```cpp
template <class T>
void sendMessage(T&& lambda) {
    Msg msg = prepareMessage();
    lambda(msg);
    send(msg);
}

// usage:
sendMessage((&)[auto& msg]{
    msg.field = ...;
});
```
* Memory allocation is costly: use a pool of objects, reuse objects instead of free'ing
* Exceptions: when not throwing 0 cost, but expensive when thrown. Don't use exceptions for control flow.
* Multi-threading is best avoided for latency sensitive code.
  * Consider not sharing data, and just provide copies of data from producer to consumer (e.g. a single reader single writer queue).
  * If you have to share data, try to avoid synchronization.
* Data lookup: pull all the data you care about in the same cache line (denormalize if necessary).
* Fast associative containers (possibly a combination of chaining and probing (? or more like another level of indirection))
* `__attribute__((always_inline))` and `__attribute__((noinline))`
* Keep the cache hot: the full hot path can be only exercised very rarely, you cache likely has been trampled by non-hot-path data and instructions. Try code warming (also trains the branch predictor correctly). Chances are Mellanox and Solarflare cards even supports this.
* Don't share L3 (or partition L3). Turn off all but 1 core. If you have all cores enabled, choose your neighor carefully / move noisy ones to different NUMA pack from you.
* Placement new used to be slightly inefficient (an extra nullptr check on the pointer you give it).
* Small string optimization in gcc >=5.1
* Static local variable initialization guaranteed to be only -- even in case of multithreaded programs (i.e. a lock is introduced)
* `std::function` can do small function optimization, but may allocate for large enough closure.
* glibc `std::pow` can be slow.
* avoid system calls.

### Measurements

* Profiling: sampling profilers, instrumentation profilers
* Benchmarking: google benchmark

Set up a production-like environment and measure end-to-end time, may not be easy to set up.

# Rich Code for Tiny Computers: A Simple Commodore 64 Game in C++17

[talk](https://www.youtube.com/watch?v=zBkNBP00wJE)

lambdas with/without bindings, anonymous namespace functions, objects/methods, function calls, structured bindings, variadic recursive templates, standard algorithms, etc, can be zero-runtime-overhead abstractions.

const can affect optimization decisions.

well defined object lifetime is the greatest strength of C++.

Straightforward logic, little branching.

# Spectre: Secrets, Side-Channels, Sandboxes, and Security

[talk](https://www.youtube.com/watch?v=_f7O3IfIR2k)


