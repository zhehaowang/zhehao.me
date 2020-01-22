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

(_why does 3 layers of cache imply 12 loops when doing tiling?_)

(_why does the recursive version look like that in slides?_)