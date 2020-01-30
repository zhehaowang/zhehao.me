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

### Assembly language and computer architecture

* Preprocessing (`clang -E`, gives .i), simple text replacement
* Compiling (`clang -S`, gives .s assembly code),
* Assembling (`clang -c`, gives .o object file), roughly one-to-one to the `.s`. `objdump -S ` produces disassembly of machine code (especially if binary is compiled with debug symbols)
* Linking (`ld`, gives the final executable)

general purpose registers (x86-64 64 bits; rax - 64 bit name, eax - 32 bit name, ax - 16 bit name, ah/al - 8 bit name. Aliased)
rsp - stack pointer
rbp - base of the frame
flag register, RFLAGS, CF - carry, ZF - zero, SF - sign, OF - overflow
program counter register
xmm, ymm vector registers

Instructions

addl %edi, %ecx : add the two, store in ecx (dest comes second, at&t syntax (used by objdump etc). Intel syntax, dest comes first)
subl %edi, %ecx : ecx - edi => ecx

mov (copy)
cmov (conditional move)
movs, movz (sign or zero extension, when moving from a 32bit register to a 64bit register)
tes, jmp, j<condition>,
call, ret

Opcode suffix
movq -16(%rbp), %rax. 
q: quad words, 8 bytes.
l or d, double word
b: byte
w: word
s: single precision
d: double precision
t: extended precision

Sign extension / zero-extension
movzbl: z - extend with zeros, first is byte long, second is double word
movsbl: s - preserve the sign, first is byte long, second is double word

cmpq $4096, %14: q - double word   (sets flag in flags register)
jne: the jump should only be taken if the arguments of the previous comparison are not equal (by looking at flags register)
Condition codes: a, ae, c, e, ge, ne, o, z

Addressing modes

at most one operand may specify a memory address.
* direct addressing modes
  * immediate, $172 constant
  * register, %rcx (single cycle)
  * memory, movq 0x600, %rdi (hundreds of cycles)
* indirect addressing mode
  * register indirect, movq (%rax), %rdi
  * register indexed,  movq 172(%rax), %rdi
  * instruction pointer relative, movq 172(%rip), %rdi
* base indexed scale displacement, most general form of x86 addressing
  * movq 172(%rdi, %rdx, 8), %rax : base + index * scale + displacement : displacement(base, index, scale)

jmp can take a label, exact address, or relative address

Assembly idioms
* xor %rax, %rax: clear %rax
*
```s
test %rcx, %rcx     ; test is bitwise-and, discard the result, preserving the RFLAGS register (sets 0 if 0)
je 400c0a           ; this means jump if %rcx holds the value 0
...
test %rax, %rax
cmovne              ; conditional move if %rax holds non-0
```
* `nop`, `nop A`, `data16` are all x84-64 ISA no-op instructions
```
data16 data16 data16 nopw %cs:0x0(%rax, %rax, 1); the effect is do nothing. This is typically compiler doing instruction alignment optimization
```

Floating point and vector hardware
* SSE uses two-letter suffixes (ss, sd: one single-precision / double-precision floating point value; ps, pd: vector (packed / vector) single-precision / double-precision value)
* SSE / AVX / x87 opcodes; generally AVX, AVX2, AVX3 extends the support in SSE (3 operands, wider vector registers)  (`addpd`, floating point packed SSE; `paddq`, integer packed SSE; `vaddpd`,  `vpaddq`, AVX instructions)
* Vector units do SIMD, processor issues the same instruction to all vector units. They act in lock step.

lea load effective address, do the calculation but don't actually load memory, sometimes used to do +- arithmetic

Architecture
* Simplified: five stage processor. Instruction Fetch (IF), Instruction Decode (ID), Execute (EX), Memory (MA), Write back (WB). These are stacked together as a pipeline.
* Intel Haswell microarchitecture has 14-19 pipeline stages

* Computer architects aim to improve processor performance by two means: parallelism (instruction-level parallelism (ILP), vectorization, multicore), and locality (e.g. caching).
* ILP, try to avoid stalls due to dependency hazards:
  * structural hazards, two instructions attempt to use the same unit at the same time,
  * data hazard, when the result of an instruction depends on the result of a prior instruction in the pipeline,
  * control hazard: the next instruction can be true / false branch of a conditional jump
* In data hazards, we have
  * true-dependence (2nd instruction reads what 1st writes),
  * anti-dependence (2nd writes what 1st reads),
  * output-depence (1st and 2nd both write to the same location)
* Complex operations, integer / floating point div are variable cycles, integer mult, floating point add - 3 cycles; floating point multiply, fused-floating-point-multiply-add 5 cycles. Separate functional units are usually introduced for complex operations such as floating point arithmetic. Fetch multiple instructions at the same time to keep different functional units busy.
* Bypassing and renaming for data hazard
* Branching prediction and speculative execution for control hazard. On Haswell, a mispredicted branch costs about 15-20 cycles.

### C to Assembly

##### C code to LLVM IR

`clang -S -emit-llvm`, emits llvm IR, before the IR gets translated into assembly.

`<Destination Operand> = <Op Code> <Source Operands>`
C-like, in unlimited registers (like C vars), no implicit RFLAGS, explicit types (e..g `i64`)

Arrays `[<number> x <type>]`
Vectors `< <number> x <type> >`
Structs `{<type>, ...}`
Pointers `<pointer>*`

Function parameters implicitly `%0, %1, ...`

Basic blocks and control flow graphs

Conditional `br <predicate> <true label> <false label>`; `br <unconditional label>`

The **static single assignment** (SSA) invariant: each instruction defines the value of a register at most once.

`phi` instruction arises commonly when you are dealing with loops, as loop induction variable changes when you execute the loop.

`%9 = phi i64 [%14, %8], [0, %6]`, depending on from where you enter this block, the register 9 can have different values. From block 6: register 9 = 0. From block 8: register 9 = register 14.

`phi` instruction doesn't map to particular assembly instruction, it exists to solve a representational problem in LLVM.

LLVM IR attributes
* align n, describes alignment of read from memory
* noalias (restrict), readonly (const)

##### LLVM IR to assembly

LLVM IR is structurally similar to assembly.

The compiler then needs to
* Select actual assembly instructions.
* Allocate which general purpose registers to hold values.
* Coordinate function calls.

Layout of a program / memory segments (high --> low virtual address)
```
+---------------------------+
| stack (grows down)        |
+---------------------------+
| heap (grows up)           |
+---------------------------+
| bss  (uninitialized data) |
+---------------------------+
| data (initialized)        |
+---------------------------+
| text (code)               |
+---------------------------+
```

Assembler directives: segment directives, storage directives, scope and linkage directives

Stack: return address, register states, local vars and function params that don't fit in registers

Linux x86-64 calling convention:
* Organizes stack into frames, `%rbp` points to the top of the current stack frame, `%rsp` points to the bottom of the current stack frame.
* `call` pushes instruction pointer `%rip` onto the stack and jumps to the operand (address of the function), `ret` pops `%rip` from the stack and returns to the caller.
* callee saves registers `%rbx` `%rbp` `%r12-%r15`, all other registers are caller-saved.
* `%rax` return val, `%rdi, %rsi, %rdx, %rcx, %r8, %r9` first to sixth args, `%xmm` for floating point args, etc


In particular, a stack frame when A calls B calls C
```
+---------------------------+ ---------------
| args from A to B          |              |
|    (linkage block)        |              |
+---------------------------+              |
| A's return address        |              |
+---------------------------+           B's frame
| A's base pointer          |              |
+---------------------------+ <== %rbp     |
| B's local vars            |              |
+---------------------------+              |
| args from B to C          |              |
+---------------------------+ <== %rsp ------
```

### Multicore programming

By Moore's law, transistor count is still growing (number of transistors doubles every two years).
But clock speed is bound at about 4GHz.

Leakage current. Transistors kept getting smaller but we are unable to keep reducing the voltage while having reliable switching. Because of being unable to reduce voltage, power density would grow to nearly that of a nuclear reactor.

The extra transistors are consequently put into multiple cores.

##### Chip multiprocessor / multicore architecture

Cache coherence, making sure the values in different processor caches are consistent across updates.

MSI protocol is one protocol to manage cache coherence.
Each cache line (64B) is labeled with a state.
* Modified, no other caches contain this block in M or S states
* Shared
* Invalid, same as cache block not being in the cache

Before a cache modifies a location, the hardware first invalidates all other copies.
(When a processor wants to set a cache line currently in S state, it turns that cache line in other processors into I state, and set this cache line to M state. M or S states can be read from directly, I state means this block has to be fetched from main memory or another processor.)

Frequent invalidation by multiple processors (hardware is going to guarantee ordering) lead to a performance bottleneck, invalidation storm.

##### Concurrency platforms

`pthread_create`, `pthread_join` (waits for the specified thread to finish)

Creating a thread typically takes `>10^4` cycles. (thread pool can help)
`pthread` api can get in the way of your business logic, hurts modularity / simplicity, needs argument marshalling (mapping arguments to a struct and pass by `void*`), etc.

Threading building block. Intel C++ library. Task abstractions, concurrent containers, parallel for and reduce as templates. Locks and atomic updates.

OpenMP. Linguistic extension in the form of compiler pragmas to specify which parts to parallelize. Underneath is pthreads.

Cilk. Linguistic extension. (`cilk_for` (have to be applied on independent iterations, otherwise result might be wrong. `cilk` does not check for independence between iteractions, a separate Cilk race detector can do `-fsanitize=cilk`, if the serial elision version can possibly give a different result than the parallel version), `cilk_spawn`, `cilk_sync`, similarity with `async/await` in JS?). Theoretically efficient work-stealing scheduler. Hyperobjects.

Cilk reducers can be defined for monoids, algebraic structures with an associative binary operation and an identity element.

Cilk doesn't command parallel execution, they grant permission for parallel execution (expresses logical parallelism).
The Cilk runtime scheduler (which the binary links) maps the executing program onto cores dynamically at runtime.
The serial elision version (`#define cilk_for for; #define cilk_spawn; #define cilk_sync`) is a viable program.

Cilk sanitizer and multicore scalability tooling.

### Races and parallelism

Race conditions are the bane of parallelism.

Determinancy race: two logically parallel instructions access the same memory location and at least one performs a write (read-write, write-write).

Two sections of code are independent if no determinancy race between them.

Depending on alignment and compiler optimization level, updating `x.a` and `x.b` may have a race condition (similar goes for bitfields)
```c
struct X {
    char a;
    char b;
} x;
```

Cilk execution model and computation DAG.
Strand (vertice, a sequence of instructions not containing a spawn, sync or return from spawn); spawn, call, return and continue edges; 

**Amdahl's law**, if 50% of your program is parallel and the other 50% is serial, you can't get a more than 2x speedup no matter how many cores you have to run your program.

##### Quantifying parallelism

* Count the number of nodes in an execution DAG that has to execute sequentially. This gives a pretty loose upper-bound.
* `T_p` execution time on P processors. `T_1 = work`, `T_infinity = span` (the longest sequential path)
  * `T_p >= T_1 / p`, work law
  * `T_p >= T_q >= T_infinity`, where `q >= p`, span law
* Series composition, `T_1(A Union B) = T_1(A) + T_1(B)`, `T_infinity(A Union B) = T_infinity(A) + T_infinity(B)`
* Parallel composition, `T_1(A Union B) = T_1(A) + T_1(B)`, `T_infinity(A Union B) = max(T_infinity(A), T_infinity(B))`
* `speedup = T_1 / T_p`, `T_1 / T_p < P` sublinear speedup, `T_1 / T_p = P` perfect / linear speedup. Superlinear speedup is possible, but not in this model due to work law.
* `parallelism = T_1 / T_infinity`, the number of processors if perfect speedup (by the laws above). Using more than `parallelism` number of cores would only yield marginal gains. Cilk scale calculates and plots this.

Example: parallel `quicksort` (derive this by looking at overall complexity (work) and sequential path complexity (span)) and parallel recursive fibonacci.

Work-efficient parallel algorithm.

##### Scheduling theory

Cilk uses a distributed scheduler.
It maps strands onto processors.

Greedy scheduler.

* Complete step: number of ready nodes (all dependencies have been finished) N > number of processors P. Greedy scheduler runs any P strands out of the N ready ones.
* Incomplete setp: run all of them.

* Greedy scheduler achieves `T_p <= T_1 / p + T_infinity` (max number of complete steps + max number of incomplete steps).
* Corollary: any greedy scheduler achieves within a factor of 2 of optimal.
* Corollary: any greedy scheduler achieves near-perfect linear speedup whenever `T_1 / T_infinity >> p`. `T_1 / P T_infinity`: parallel slackness.

Cilk's work-stealing scheduler achieves `T_p = T_1 / p + O(T_infinity)`, in practice usually `T_p = T_1 / p + T_infinity`.
Each processor maintains a deque of ready strands, whenever a processor runs out of work, it steals a random processor's work.

Corollary: with sufficient parallelism, workers steal infrequently and we have a linear speed-up. 

Cactus stack. Theory on stack space bound. (_?_)

### Analysis of multi-threaded algorithms

The Master Method for divide-and-conquer algorithm complexity analysis. Proof.

The more general solution is the Akra-Bazzi method.

Example: Cilk for matrix transpose. Recursive spawn and sync when doing a `cilk_for` outer loop, work: `theta_(n^2)`, span: `theta_(n)`, parallelism: `theta_(n)`. When doing a `cilk_for` on inner loop as well, span becomes `theta_(lg_n) + theta_(lg_n) + 1` where the first two terms are loop control.

Coarsening parallel loops with Cilk pragma, reducing parallel function call overheads.

Work and span with spawn and return overhead considered. Vector add `cilk_for` example. G: grain size. I: one addition. S: spawning and returning overhead.
`T_1 = n I + (n / G - 1) S`
`T_infinity = G I + lg(n / G) S`

We want `G >> S / I` such that `T_1` is dominated by the actual work, the first term.

Parallel performance tips
* Minimize the span to maximize parallelism. Generally you want 10x more parallelism than processors for near perfect linear speedup
* If you have enough parallelism, trade some of that to reduce work overhead
* Use divide-and-conquer recursion or parallel loops rather than spawning one small thing after another
* Ensure work per spawn is relatively large
* Generally better to parallelize outer loops than inner loops 
* Watch out for scheduling overhead

Matrix multiplication example.
Row-major representation.
Strassen's worthwhile doing for sufficiently large matrices.

### What compiler can and cannot do

C/C++/Rust/Swift/Haskell/Julia == compiled ==> LLVM IR == (often times) LLVM optimizer ==> optimized LLVM IR

The optimizer works in a series of transformation passes (in a certain order).

`-Rpass=<string>` get reports on what the specified pass regex did.

A lot of the transformation corresponds with the new Bentley rules discussed earlier, out of which compiler rarely or does not do
* creating a fast path (logic)
* coarsening (functions)
* sentinels (loops)
* most data structure optimizations (it does a different set of data structure optimizations, like)
  * register allocation
  * memory to registers
  * scalar replacement of aggregates
  * alignment
And restrictions apply when:
* ordering tests (logic)
* Combining tests (logic)
* Loop fusion (loop)

Most compiler optimization happens in IR, but not all, e.g.
* `uint * 8` in C ==> `l shift by 3` in IR ==> `lea (,%src_reg,8), %dst_reg` in asm
* `uint * 15` in C ==> `* 15` in IR ==> `lea (%reg1,%reg1,4), %reg2; lea (%reg2,%reg2,2), %reg3` in asm
* `uint / 71` in C ==> `/ 71` in IR ==> `magic_number = (2^38 / 71 + 1); mul magic_number; r shift 38` in asm, since multiply is faster than division.

Hacker's Delight. Full of bittricks.

##### Some example optimizations

Optimizing a scalar value:
* replace stack-allocated variables with the copy in the register (hence the often times seen `optimized out` in gdb when printing stack variables) (O0 --> O1 does this)

Optimizing a structure:
* you may not be able to store all the data of a structure in registers, but optimizing it as a pack of scalars and applying the technique above still works (O0 --> O1 does this)

Optimization often makes generated IR look much simpler.

Optimizing function calls (the following usually happens at O2 or higher):
* Inlining.
  * Moving the body of the function to the caller, which often enables more optimization, e.g. removing the need to pack data into a structure to be passed and immediately unpacked in the callee. These try to eliminate the cost of the function abstraction
  * Why not eliminate all function calls?
    * Recursion can be hard to inline (except recursive tail call).
    * Compiler cannot inline a function defined in a different translation unit, except when using whole program optimization
    * Bloating code size, hurting instruction cache locality.
  * Compiler makes a best guess based on heuristics such as function size to decide what functions to inline. `__attribute__((always_inline))` `__attribute__((no_inline))` to tell compiler what to inline. **Link Time Optimization** (LTO) enables whole program optimization. The `inline` keyword provides a hint.

* Loop optimization
  * Accounts for most of execution time in programs
  * Hoisting (loop invariant code motion / licm)

Consider this code
```cpp
void times(double* A, double* B, int mult, int length) {
    // vector arithmetic, A = A + B * mult
    for (int i = 0; i < length; ++i) {
        A[i] += B[i] * mult;
    }
}
```
Does this vectorize?
Answer is yes and no.
Compiler generates two branches for whether A and B overlap due to uncertainty about aliasing: does A and B overlap?
If yes, no vectorize; if no, vectorize.

Many compiler optimizations act conservatively when memory aliasing is possible.
Compiler tries hard to analyze alias: clang uses metadata to track alias information derived from multiple sources, but in general alias analysis is undecidable.
You can help by annotating your pointers `restrict`. `const` helps, too.


`__restrict` keyword can give the compiler more freedom to do optimizations, knowing this is the only pointer pointing to the data.

Signed and unsigned shift (_what does signed shift do?_)

(Log time Fibonacci number algorithm)

(_why does 3 layers of cache imply 12 loops when doing tiling?_)

(_why does the recursive version look like that in slides?_)