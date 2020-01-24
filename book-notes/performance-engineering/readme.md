# Performance Engineering MIT 6.172

### Introduction

Performance is often not the No.1 concern of building software.
Performance is the currency / budget of computing, you use performance to buy easy-to-program, security, testability, usability, etc.

Premature optimization is the root of all evil. (taken out of context)

More computing sins are committed in the name of efficiency (without necessarily achieving it) than for any other single reason, including blind stupidity.

First rule of performance optimization: don't do it. Second rule: experts only.

Clock speed plateau'ed in 2004. (Due to power density growth, the dynamic power is less of a concern than static circuit leakage whose heat generated we can't yet handle well)
Vendors switched to a multicore solution.
Multicore parallelism, vector units, GPU, steeper cache hierarchy all made performance enigneering relevant again.

The big question: how do we write software to effectively leverage the complex modern hardware.

L1, L2 caches private to the processor, shared across processors L3/LLC cache.

### Example: matrix multiplication

Naive python 4096 x 4096 matrix multiplication (iterate by dest row, column, index) takes about 6 hours on a Haswell Intel Xeon E5-2666 v3 (18 cores, 8 double precision operations per core per cycle, including fused-multi-add, peak 836 GFLOPS).

The naive solution needs `2n^3` operations, `2^37` total. Python is at 0.007 GFLOPS, 0.001% of peak.

Same code in Java takes 46 minutes, 9x speed up from python.
In C it takes 19 minutes, 18x from python.

Why Python is slow
* Python is interpreted. Interpreters are versatile but slow. E.g. dynamic code alteration at the cost of performance.
* C code compiles directly to machine code, and compiler optimization produces highly optimized machine instructions.
* Java is compiled into bytecode, which is then interpreted and JIT-compiled to machine code. Somewhere in between, JIT invokes the compiler to compile frequently run code, and subsequent runs become faster.

With loop reordering, `(k, i, j)` or `(i, k, j)` is 18x faster than `i-last` loop order. (With a row-major-order, `j` last gives good locality)

valgrind cachegrind can measure LLC miss rate.

Clang O2/O3 in this case can give another 3x.

Profile guided optimization.

With C, Clang O3, best loop ordering, we get 0.3% of peak performance.

Parallel loop `cilk_for` `(i, k, j)` with `cilk_for` on `i` loop is fastest, both `i` and `j` is worse, and parallel `j` loop is worst.
This is almost another 18x, bringing us to 5% peak.

Doing it by blocks (tiled matrix multiplication / tiling) gives another 1.7x.

Recursive parallel matrix multiply gets great numbers on cache but we are much slower, because of function call overhead.
Coarsening the base condition yields better result, bringing us to 12% peak.

Vector stream doing SIMD (single instruction multiple data stream) `-Rpass=vector , -march=native` in clang

Floating point operations are not associative (`(a * b) * c != a * (b * c)`). `-ffast-math` allows reordering of floating point operation, at the cost of producing potentially different results.
`-fast-math -march=native` gives another 2x.

C intrinsic instruction allows you to write platform specific code vector instructions without going through compiler optimization.
AVX intrinsics can do another 2x, bringing us to 41% peak.
Intel MLK does about this much, and for general size matrices, 50000x from where we started in Python.

This course allows you to print performance money, and focuses on multicore performance

GPU, file system, network performance are really important in real life, and some lessons can be learnt from optimizing CPU performance / memory access pattern.

### Bentley rules for optimizing work

Bentley's book on performance optimization.

* Data structure
  * Packing and encoding
  * Augmentation (store more metadata / auxiliary data)
  * Precomputation (e.g. computing binomial coefficient / n-choose-k, or precompute a pascal triangle. Correspondence with binomial coefficients.)
  * Compile-time initialization, metaprogramming (write a program that writes your program).
  * Caching
  * Sparsity (avoid storage and computation on zeroes; Compressed Sparse Row (CSR, storage is (number of rows + number of non-zero elements)) for storing matrix with lots of 0s. Matrix multiplication will change as well. Similarly, a static sparse graph can be represented in CSR, if we think of the matrix as representing graph connectivity.)

* Logic
  * Compile-time constant folding and propagation
  * Common subexpression elimination
  * Algebraic identities (replace more expensive algebraic expressions with equivalents that cost less)
  * Short circuiting
  * Ordering test (more frequent that leads to short circuiting -> least frequent)
  * Fast path
  * Combining tests

* Loops
  * Hoisting (avoid recomputing loop-invariant repeatedly within the loop)
  * Sentinels (dummy value in a data structure to simplify boundary conditions checking, e.g. loop exits. Interesting sum of integers overflow check.)
  * Loop unrolling (full and partial, reduce number of control instructions, easier pipelining, enables more compiler optimizations; overdoing this might pollute instruction cache, having bigger loop body initially also diminishes this)
  * Loop fusion / jamming. (Combine multiple loops with the same index range)
  * Eliminating wasted iterations (modify loop bounds to avoid executing over empty loop bodies)

* Functions
  * Inlining
  * Tail recursion elimination: replace the tail recursive call of a function with a branch to save one function call overhead (_tail call elimination seems easier to follow, tail recursion elimination seem to do the same thing tho the quicksort example_)
  * Coarsen recursion: increase the size of base case to reduce function call overheads

**Advice**
* Avoid premature optimization
* Regression performance testing.
* Reducing work doesn't necessarily reduce running time, but is often a good heuristic.
* Look at assembly to decide if the compiler optimized something away.

### Bit hacks

Signed representation (same rule as unsigned representation sigma of geometric series, except treating MSB as - MSB * `2^(n - 1)`.

Two's complement identity, `x + ~x = -1`, `-x = ~x + 1`
* set the k-th bit `y = x | (1 << k)`
* clear the k-th bit `y = x & ~(1 << k)`
* toggle the k-th bit `y = x ^ (1 << k)`
* extract a bitfield `y = (x & mask) >> shift`, mask like `0b1100` and same length as x, in which case shift is the number of 0s to the right, 2.
* set a bitfield `x = (x & ~mask) | (y << shift)`, mask and shift like above, y should occupy fewer digits than the mask covers, so for safety, do `x = (x & ~mask) | ((y << shift) & mask)`
* swap. `x = x ^ y; y = x ^ y; x = x ^ y`, xor is its own inverse. This is poor at exploiting instruction level parallelism due to sequential dependency here, hence likely to perform worse than the temp variable solution
* find min. Performance issue with the default branching solution is a mispredicted branch empties processor pipeline (which the compiler is usually smart enough to optimize the unpredictable branch, but not always). `r = y ^ ((x ^ y) & - (x < y))`
  * Predictable (true/false cases are more skewed and not almost 50/50) vs unpredictable branches.
  * The one unpredictable branch in merging two sorted arrays can benefit from branchless comparison. Not necessarily the case for `clang -O3`, who does the optimization (with a branchless cmove) better than you.
* modular addition `(x + y) % n`, assuming both x and y are `[0, n)`, rather expensive division unless n is a power of 2. `z = x + y; r = (z < n) ? z : z - n` has an unpredictable branch. `z = x + y; r = z - (n & -(z >= n))`, same idea as getting min of two.
* rounding up to a power of 2
```cpp
uint64_t n = xxx;
--n;               // so that this works for n = 2^k.
n |= n >> 1;
n |= n >> 2;
n |= n >> 4;
n |= n >> 8;
n |= n >> 16;
n |= n >> 32;
++n;
```
* least significant 1 in x. `r = x & (-x)`, due to `-x = (~x) + 1`.
  *  To find the index of that least significant 1 (find `lg(n)` where `n` is a power of 2), multiply by deBruijn constant and shift according to a deBruijn sequence. We can prove for any length, there is a deBruijn sequence. This is how you would do it before a hardware instruction to do this came out.
```
e.g. k = 3
00011101
(cyclic)
000        - 0, 0
 001       - 1, 1
  011      - 2, 3
   111     - 3, 7
    110    - 4, 6
     101   - 5, 5
      010  - 6, 2
       100 - 7, 4
convert[8] = {0, 1, 6, 2, 7, 5, 4, 3}
idx == debruijn[convert[idx]]
```
* N queens count number of valid ways. Board representation in backtracking search. For this problem, a more compact solution is to use a down vector of 8 bits, two diagonal vectors of `(8 * 2 - 1)` bits each. When a field in the bit vector is 1, it means there is a queen covering that column or diagonal.
* Count the number of 1 bits in x
```cpp
for (r = 0; x != 0; ++r) {
    x &= x - 1;
}
```
Or use table look up, need to go to memory for table lookup
```cpp
static const int count[256] = {0, 1, 1, 2, 1, 2, 2, 3, ..., 8};
for (int r = 0; x != 0; x >>= 8) {
    r += count[x & 0xFF];
}
```
Or parallel divide-and-conquer, use 5 masks of `(01)_32` (01 repeated 32 times), `(0011)_16`, `(00001111)_8`, `((0)_8(1)_8)_4`, ...`x = x & m0 + (x >> 1) & m0` gives you the number of 1 bits in every 2 bits, then do `x = x & m1 + (x >> 2) & m1` (or when not needing to worry about overflow, `x = (x + (x >> 4)) & m2`) gives you the number of 1 bits in every 4 bits, do this for all masks to "fold" and get the number of 1 bits in the original number.
There is a hardware popcount instruction today as well, which is faster than the above.

[Check out more](https://graphics.stanford.edu/~seander/bithacks.html)



`__restrict` keyword can give the compiler more freedom to do optimizations, knowing this is the only pointer pointing to the data.

Signed and unsigned shift (_what does signed shift do?_)

(_why does 3 layers of cache imply 12 loops when doing tiling?_)

(_why does the recursive version look like that in slides?_)