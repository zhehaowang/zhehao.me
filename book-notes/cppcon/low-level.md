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

# 
