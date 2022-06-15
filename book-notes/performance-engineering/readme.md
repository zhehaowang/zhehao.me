# Performance Engineering MIT 6.172

### Introduction

Performance is often not the No.1 concern of building software.
Performance is the currency / budget of computing, you use performance to buy easy-to-program, security, testability, usability, etc.

Premature optimization is the root of all evil. (taken out of context)

More computing sins are committed in the name of efficiency (without necessarily achieving it) than for any other single reason, including blind stupidity.

First rule of performance optimization: don't do it.
Second rule: experts only.

Clock speed plateau'ed in 2004. (Due to power density growth, the dynamic power is less of a concern than static circuit leakage whose heat generated we can't yet handle well)
Consequently, vendors switched to a multicore solution.

Multicore parallelism, vector units, GPU, steeper cache hierarchy all made performance enigneering relevant again.

The big question: how do we write software to effectively leverage the complex modern hardware.

L1, L2 caches private to the processor, L3/LLC cache shared across processors.

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

This course allows you to print performance money, and focuses on multicore performance.

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
+------------------------------------------------------+
| stack (grows down)                                   |
+------------------------------------------------------+
| heap (grows up)                                      |
+------------------------------------------------------+
| bss  (uninitialized data, initialized to 0 at start) |
+------------------------------------------------------+
| data (initialized, global, static data)              |
+------------------------------------------------------+
| text (code)                                          |
+------------------------------------------------------+
```
Stack and heap won't hit each other in practice, as we use 64b virtual address space.

When putting huge chunks of precomputed consts to your program, your startup loading time might be higher.

Assembler directives: segment directives, storage directives, scope and linkage directives

Stack: return address, register states, local vars and function params that don't fit in registers

Linux x86-64 calling convention:
* Organizes stack into frames, `%rbp` points to the top of the current stack frame, `%rsp` points to the bottom of the current stack frame.
* `call` pushes instruction pointer `%rip` onto the stack and jumps to the operand (address of the function), `ret` pops `%rip` from the stack and returns to the caller.
* callee saves registers `%rbx` `%rbp` `%r12-%r15`, all other registers are caller-saved.
* `%rax` return val, `%rdi, %rsi, %rdx, %rcx, %r8, %r9` first to sixth args, `%xmm` for floating point args, etc


In particular, a stack frame when A calls B calls C
```
+---------------------------+ -------------+
| args from A to B          |              |
|    (linkage block)        |              |
+---------------------------+              |
| A's return address        |              |
+---------------------------+           B's frame
| A's base pointer          | <== %rbp     |
+---------------------------+              |
| B's local vars            |              |
+---------------------------+              |
| args from B to C          | <== %rsp     |
+---------------------------+ -------------+
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

### Timing and measurement

mergesort running time analysis; several peaks; machine changing clock frequency. (Dynamic Voltage and Frequency Scaling feature of processor, to reduce power consumption)

`Power` is proportional to `C V^2 f`, C is the dynamic capacitance (area of circuitry * activity (how many bits are moving)), V is supply voltage, f is clock frequency

How to reliably measure in the face of the above?

##### Quiescing systems

Before increasing the quality of the product, let's first reliably reproduce similar products first.
Go after the variance first.

If you can reduce variability, you can compensate for systematic and random measurement errors.

Source of variability: daemons, interrupts, code and data alignment (if code goes across page boundaries, then TLB miss on the page can have a big impact), thread placement (system uses core 0 to do its own stuff, so don't do your measurement on it), runtime scheduler (when multicore), hyperthreading (simultaneous multithreading, two instruction streams through the same functional unit at the same time with different registers. x1.2 speedup), multi-tenancy (other people using the system), DVFS, turboboost (when processor only has one core running, have that core run at a higher frequency), network traffic.

Unquiesced system with all the above, the worst run can be 25% slower than the fastest run.

Turning all the above off you get about the same value every run.

Making sure no other jobs, shut down daemons and crons, disconnect the network, don't fiddle with the mouse (200 interrupts / s), for serial jobs don't run on core 0 where interrupt handlers are usually run, turn off hyperthreading, dvfs, turboboost, use cpuset (manually map threads to cores) etc.

There is no way to get completely deterministic results in modern hardware, due to memory errors: when you access DRAM, it's possible an alpha particle collide with one of the bits and flips it. Hardware detects this error and uses one cycle to correct it. This effect is undeterministic. (Scheduler, branch prediction, caches, etc are deterministic algorithms)

If your change causes code alignment change, it can have big impact on your performance even though your change may seem performance-agnostic.
Changing the order in which `*.o` files appear on the linker command line can have a larger effect than going between `-O2` and `-O3`.
A program's name (ends up in an environment variables ending up on call stack) can affect its speed (e.g. cause critical data to go on two lines)!

Now compiler does a lot of alignment, e.g. for function starts to be aligned with the start of a cache line (hence changing one function won't mess with the cache alignment of another function). `-align-all-functions`, `-align-all-blocks` (as llvm labeled blocks, may bloat your binary), `-align-all-nofallthru-blocks`.

Aligned code can usually reduce variance, but may give worse performance.

##### Tools for measurement

* `time` command, you can't time something very short with time.
  * real wall clock time
  * CPU user mode (code outside kernel) time running your process
  * CPU kernel mode time running your process
* Instrument the program. E.g. `clock_gettime`, `rdtsc()`, or with compiler support (these may change the timing)
  * `clock_gettime(monotonic)`, takes about 80ns, guarantees never to run backwards. Good reliable numbers. 
  * `rdtsc()` read timestamp counter instruction, takes about 32ns. `rdtsc()` may give different answers to different cores on the machine (tsc is processor by processor basis). Sometimes `tsc` runs backwards, and `tsc` may not progress at a constant speed, hence converting clock cycles to seconds can be tricky.
  * `gettimeofday()`, ms precision, can run backwards.
* Interrupt the program. E.g. gdb and `ctrl+c` at random intervals, similar idea is used to implement Poor Man's Profiler, automate this and you get `gprof`. `gprof` samples 100 times a second.
* Exploit hardware and OS support. `perf`
  * `libpfm4` virtualizes all hardware counters: `perf stat` uses this. There are many esoteric hardware counters, often they are not well documented.
  * LLC miss * cache line size used to be how much memory was loaded from DRAM. But prefetching (fetch into cache and doesn't update LLC miss counter) is not counted.
* Simulate the program. `cachegrind` (much slower, repeatable and accurate, and your simulator doesn't necessarily model everything. It's great for cache misses.)

A good strategy is triangulation: take more than one measurement in different ways and make sure they are telling the same story.

##### Performance modeling

Have a model to interpret your numbers, don't just give or trust your numbers.
If you can't measure performance reliably, it's hard to make small changes that add up.

Suppose you measure the performance of a deterministic program 100 times on a computer with some interfering background noise. What statistics is the best way to measure raw performance?
Arithmetic mean?
Geometric mean? (arithmetic mean of the logs. Take the product of n numbers and get nth root)
Harmonic mean?
Median?
Maximum?
Minimum (best at noise rejection)?

Does it ever make sense to take the arithmetic mean of a bunch of ratios?
The ratio of the means is not the mean of the ratios. Try geometric mean.

What's a good strategy to compare two programs which one being faster, given a slightly noisy measurement?
Try n head-to-head comparisons between A and B.
Consider the null hypothesis that B beats A, calculate the p-value (if B beats A, what is the probability that we'd observe that A beats B more often than we did). If the p-value is low, we can accept that A beats B.

Fitting to a model. Overfitting.

### Storage allocation

Stack: array and pointer. Can overflow. Pointer +- when allocating or freeing. Very fast. `theta(1)` time. Limitation: you can only free the last thing you allocated.

##### Heap

For a language without garbage collector, programmer has to manually manage memory (leaks, dangling pointers, double freeing (UB). Address sanitizer (compiler instrumentation) and valgrind (virtual machine with JIT, tends to catch fewer bugs) can help).

##### Fixed size allocation

* Freelist. Every piece has the same size, unused storage has a pointer to the next unused block. (Alternatively, use a bit to tell if a piece is used).
* To alloc, grap the head of the list, modify the list head to next. To free, point the block's next to list head, then set list head to point to the block. `theta(1)` time alloc and free. Good temporal locality: last thing you freed is the first thing you allocate. Poor spatial locality: **external fragmentation** (blocks of used memory are all over the place, consequently TLB (page table cache) and disk thrashing can be problematic). 

Mitigating external fragmentation:
* Keep a free list per disk page, allocate from the freelist of the fullest page. VRAM can swap out a completely empty page. Based on the observation that two pages filled 90-10 (skewed) is better than two pages filled 50-50 (even) (higher likelihood two random accesses will hit the same page).

##### Variable size heap allocation

**Internal fragmentation**: wasted space within a block.

One variant of a memory allocator:

Binned freelist, combats internal fragmentation.
Bin `k` holds a freelist of memory blocks of size `2^k`.

When alloc size `m`, check the freelist of `roundup(lg_2(m))`, if nonempty, return the head and move the new head.
Otherwise go to the next bigger freelist, allocate from its head, break the allocated bigger chunk and attach the now smaller free ones to the earlier freelists.

`sbrk`, `mmap`: ask the OS for more memory so your storage allocator, e.g. the scheme above, can use it.

Why not never free? Given we have 64b virtual memory address space.
External fragmentation, low TLB hit rate and disk thrashing is going to be bad.

Suppose the maximum amount of heap memory used by a program is `M` and the heap is managed by a binned freelist, then the upper bound of virtual memory consumed is `O(M lg_2(M))`.

Binned freelist is `theta(1)` competitive with the optimal allocator (who knows all the requests in the future) (assuming no coalescing). (the constant factor is 6)

**Coalescing**: splicing together adjacent small free blocks into a larger block. No theoretical bounds known.

##### Garbage collectors

Three types of memory objects: roots (globals, stack objects, etc), live, dead

In general in order for garbage collector to work, you need to have garbage collector identifying pointers. This requires strong typing, C's weak typing on ptrs/ints won't do.
Also need to prohibit doing pointer arithmetic.
Hence C can't have a general purpose garbage collector that works well.

* Reference counting. A cycle (or those referenced by something in a cycle) is never collected. Efficient and simple to implement. 
* Mark and sweep. A Graph (V, E) of all objects and directed edges representing one referencing another. Mark: BFS from roots, mark all reachable from roots. Sweep: scan over memory to free all unmarked objects. Scanning over can be expensive and this doesn't deal with fragmentation.
* Stop and copy. Instead of BFS and marking, copy the object over as we do BFS. After BFS, free all the source. This addresses fragmentation. This invalidates pointers, to address this we can instead keep a pointer in the source to the copied destination when removing from the source, and adjust the pointers after copying. This is linear to the number of objects, pointers and space you are copying over.

Topics in dynamic memory allocation include buddy system, variants of mark-and-sweep, generational garbage collection (scanning over younger objects first, since often objects are short lived), real-time garbage collection (correctness issues when garbage collectors don't stop the world and run in background), multithreaded garbage collection, 

### Parallel storage allocation

##### `mmap` and `malloc`

`void* malloc(size_t s)`, `void* memalign(size_t a, size_t s)`, `void free(void *)`

`mmap` system call usually treat some file on disk as part of memory, so that when you write something to memory it also backs up on disk.
`mmap` the kernel finds a contiguous unused region in address space of the application large enough to hold the requested size, modifies page table, and creates necessary virtual memory management structures within the OS to make user's access to this area legal so that accesses won't result in a segfault.

`mmap` is **lazy** in that it does not immediately allocate physical memory for the requested allocation. Instead it populates page table pointing to a special zero page and marks that as read only. The first write will then result in a page fault, upon which OS allocates a physical page, modifies the page table, and restarts the instruction.

You can `mmap` a TB virtual memory from a machine with a GB of RAM, and a process can die from running out of physical memory after the `mmap` call.

`malloc` (C library call) uses `mmap` (system call) to get more memory from OS.
It's going to attempt to satisfy user requests for heap storage by reusing freed memory whenever possible, and when necessary it uses `mmap` to expand the size of user's heap storage.
`mmap` is heavy weight and works on a page granularity.

##### Address translation

**Address translation**: given a virtual address (virtual page #, offset), look in **page table** for (virtual page #, physical frame #) mapping, physical frame # corresponds to a location in DRAM. +offset on that location gives you the physical memory location.

If a virtual page does not reside in physical memory (entry in page table but the physical page has been moved out), a page fault occurs, in which case the OS checks if the process has permission to look at the memory region, if so, it places the entry into page table, otherwise it segfaults.

If a translation is not found for the virtual address, the virtual address is invalid and OS sends a segfault to the offending program.

Page table lookup is expensive, and its lookup results are cached in TLB.

When r/w on the translated physical memory, we first check if it's loaded in CPU cache. If not, turn to DRAM.

Modern hardware lets you do TLB access and L1 access in parallel, so L1 uses virtual memory address.

##### Stack of parallel execution

Serial C/C++ program stack is a walk on the call tree, parents can pass pointer to stack location to children, but not other way round. For children to allocate something for parent, it can do so on the heap, or on parent's stack.

**Cactus stack** supports multiple views of the stack in parallel.
Say A spawns B and C in parallel, then A's stack space is not copied.
Cactus stack can be implemented by having each parallel spawn work off another stack (pool of linear stack)
```
+---+
| A |
+---+
|   |
|   |
+---+
 |
 |------|
+---+  +---+
| B |  | C |
|   |  |   |
+---+  |   |
|   |  |   |
+---+  +---+
```
Or use heap to implement: use heap to allocate a stack frame.
Space bound of P-workers: `S_p <= p * S_1`, where `S_1` is the stack memory usage of a serial execution.
Hea-based cactus can have poor interoperability with compiled library code.

Parallel divide and conquer matrix multiplication example.

##### Heap allocator metrics

Allocator **speed**: optimize for small blocks (larger blocks this cost of allocation gets amortized away).

User footprint (U), allocator footprint (A)

**Fragmentation** (F) = Allocator Footprint / User Footprint

Allocator Footprint grows monotonically for many allocators, as they don't necessarily give back the page to the OS due to the cost.

Fragmentation of binned freelist is `F_v = O(log_2(U))`

**Space overhead**: allocator bookkeeping data structure overhead

**Internal fragmentation**, **external fragmentation**, **blowup** (parallel allocator overhead over a serial allocator).

##### Parallel heap allocation strategies

* Global heap (default `malloc` strategy). All threads share the same heap, accesses are mediated by a mutex or lock-free synchronization. Low blowup: 1, slow: synchronization overhead, lock contention can hurt scalability.
* Local heap. Fast: no synchronization required, suffers from memory drift, allocators aren't smart enough to reuse space from another heap. Potentially unbounded blowup. (Each thread has its own heap and freelist, when one thread frees, the freed space goes to the freelist of freeing thread, not the allocating thread)
* Local heap with local ownership, such that when freed, the freed space goes to the freelist of the allocating thread. Freeing remote objects then requires synchronization. Local allocation and free is fast. `Blowup <= p`. Resilience to false sharing.

**False sharing**: when multiple processors access different locations but they happen to be on the same cache line. This increases cache line thrashing. General approach to avoid this is padding.
**True sharing**: when multiple processors access the same location.

* Hoard allocator: P local heaps, 1 global heap. Memory organized into superblocks of size S. Superblocks are moved between local heaps and global heaps: local heap gets and returns from global heap, global heap gets from OS. Fast, scalable, bounded blowup, resistence to false sharing. Bounded blowup.

* jemalloc
* SuperMalloc

### Cilk runtime system

Cilk expresses logical parallelism.

Cilk library / runtime system.

##### Design goals

The problem (think from the execution DAG):
A single worker needs to execute the code serially, steal (thieves needs to pick up state where the victim processor left off; thieves also need to handle mixture of spawned and called functions; this should also have minimal overhead), sync (wait only on nested subcomputation), cactus stack for parallel workers.

Linear speedup is desirable, for which we need ample parallelism (`T_1 / T_infinity >> p`) and high work efficiency.

Cilk runtime system adopts work first principle: optimize for the ordinary serial execution, at the expense of some additional computation in steals.

##### Implementing worker deque

The worker deque is an external structure with pointers to stack frame.
Cilk worker maintains head and tail to the deque.
Stealable frames maintain a local structure to store information needed to resume when stolen.

Spawning function, spawn helper, detach, `setjmp`.

##### Stealing computation

Deque synchronization. THE protocol (thief always grabs the lock, worker pops the deque optimistically). `longjmp` stealing register states, returns with the specified value from the `setjmp` that set the buffer area with (_is this similar to `fork`?_).

Cactus stack (thief's `%rbp` points to victim spawning function stack, `%rsp` points to the start of its own stack. Disables repurposing `%rbp` compiler optimization)

##### Sync

Full frame trees keep track of which subcomputations are outstandings (pointers to parents frames and number of child frames.)
Processors work on active full frames.

### Cache efficient algorithms

Processor specification, e.g.
L1d, L1i (private to a processor core): 32KB, assoc 8, 2ns
L2 (private to a processor core): 256KB, assoc 8, 4ns
LLC (shared across processor cores): 30MB, assoc 20, 6ns
Main memory: 50ns
64B line

Cache coherenece, e.g. MSI cache protocol. Verification / correctness proof.

##### Cache design

**Fully associative cache**: a cache block can reside anywhere in the cache.
To find a block in cache, search the entire cache for the tag (high bits of the address space). This may be slow.
Eviction policy, e.g. LRU.

**Direct-map cache**: each block in the address space can only go to a determined location in cache (a block's **set**.
Given an address, cacheline size of `B`, cache size of `M`, use the last `log_2(B)` bits to decide offset, middle `log_2(M / B)` bits to decide the set, where in the cache this block can go to, and higher `address_space_length - log_2(M)` bits to decide the tag, check this to tell whether a physical address is in cache or not).

This is faster in telling whether a line in cache or not. This is bad in conflict miss: cache has empty space, but you may have to keep evicting things because they map to the same set.

**Set associative cache** (hybrid): each block in physical memory can go to any space in the set it maps to. Each set has a few empty spots.
Given an address, cacheline size of `B`, cache size of `M`, `k`-way associativity, use the last `log_2(B)` bits to decide offset, `log_2(M / kB)` for the set, and `address_space_length - log_2(M / k)` for tag.

To find a block in cache, `k` locations within the set must be searched.

The assoc / associativity above is the number `k`.

##### Cache misses

* **Cold miss**, first time the cache block is accessed.
* **Capacity miss**, the previous cached copy would have been evicted even using a fully associative cache. Improve spatial and temporal locality to bring this down.
* **Conflict miss**, as described in direct-map cache, conflict in the same set while cache still has space. (think accessing a column in row-major-order matrix; consider padding or copying a smaller section over)
* **Sharing miss**, true sharing miss, multiple processors are accessing the same data on the same cache line, at least one is modifying. False sharing miss, two processors accessing different data on the same cache line, at least one is modifying.

##### Ideal cache model

Omniscient about future accesses (and replacement), fully associative.

Measures work W (all operations) and cache misses Q. Ideally we want low Q without incurring too much overhead on W.

**LRU lemma**: suppose that an algorithm incurs Q cache missies on an ideal cache of size `M`, then on a fully-associative cache of `2M` that uses LRU, it incurs at most 2Q cache misses.
The implication is that optimal cache and LRU cache differs at most by a constant factor.

First theoretically good algorithm, then engineer for performance.

Lemma: even if our data is divided into segments (of arbitrary length), as long as the average length of the segments is large enough (i.e. larger than a cache line), then the number of cache misses is just a constant factor (3) worse than reading a single array (of all segments combined).

##### Example: matrix multiplication cache analysis.

Tall cache assumption (for analysis): the number of cache lines is bigger than the size `B`. (`B^2 < cM` where `c <= 1` is a constant)
(TLB as a cache may not satisfy this assumption)

Under the above assumption and `(i, j, k)` iteration sequence, `theta(n^3)` cache misses as dominated by cache misses in B.
`(i, k, j)` gives `theta(n^3) / B`

**Tiling** into `(n / s)^3` (6 nested for loops) subproblems optimizes the above by another `sqrt(M)`, tune `s` such that `s = theta(sqrt(M))`.
The `s` is not portable in that it is cache-size dependent.

Recursive divide-and-conquer matrix multiplication (coarsen the leaf level problem fits in cache) gets the same asymptotic number of cache misses.
The resulting algorithm is **cache-oblivious**, in that there is no parameter `s` to tune.

The best cache-oblivious code to date work on arbitrary rectangular matrices and perform binary splitting (instead of 8-way) on the largest of i, j and k.

Parallel analysis.
Conclusion with Cilk: to optimize the cilk-paralleled version's cache performance, optimize the cache performance of the serial version.

### Cache oblivious algorithms

Code has no knowledge of cache size. Cache-aware algorithms, code has knowledge of cache size, etc.

##### Example: heat diffusion algorithm

Differential equation.

1-dimension diffusion differential equation
```
du / dt = alpha * (d^2 u / d x ^ 2)
```

Finite-difference approximation method

Stencil computation (the above becomes a 3-point stencil, where each time t moves up, `u[t + 1][x]` is determined by `u[t][x - 1]`, `u[t][x]`, `u[t][x + 1]`).
* Looping approach.
* Recursive divide and conquer approach, trapezoid approach. Time and space cuts.
* Parallel space cuts. Multiple non-overlapping trapezoids at the same time.

Impediments to speedup
* insufficient parallelism (Diagnosed with work and span analysis)
* scheduling overhead (Diagnosed with work and span analysis)
* lack of memory width (Run p copies of the serial program in parallel to see if they slow down)
* contention (locking and true/false sharing) (Harder to diagnose)

##### Example: cache efficient sorting

* Merge: `theta(n)` work complexity, `theta(n/B)` cache complexity
* Mergesort
  * `Q(n) = 2 Q(n / 2) + theta(n / B)`. This gives `theta(n log_2(n / M) / B)`, where we stop at leaves of size `n = cM` for `c <= 1`
  * Improve with multi-way (`R`-way) merging (tournament tree). Same work. This gives another `log_M` cache saving. This is no longer cache oblivious as `R` is cache-size related.

Funnelsort, optimal cache oblivious sorting. 

Cache oblivious data structures

### Non deterministic parallel programming

Define determinism. Every memory location is updated with the same sequence of values in every executon.
There are other definitions of determinism.

Never write non-deterministic parallel program. If you must, come up with a test strategy.
Though you might get better performance sometimes with non-deterministic programs.

Good testing strategies:
* turn off nondeterminism. malloc randomized address for security (you can't deterministically exploit buffer overflow). There is compiler flag / debugging mode that does not have this on.
* record and replay
* encapsulate the nondeterminism
* analysis tools

##### Types of races

Example: concurrent chaining hash table
* Standard approach to this is to make a sequence of instructions atomic (a critical section). Implemented with a mutex.
```
slot = hash(x->key);
// lock
x->next = table[slot].head;
table[slot].head = x;
// unlock
```

Recall: **determinacy race**. With mutexes, by definition you still have a determinacy race (concurrent access on the lock). (Hence any program with a mutex are nondeterministic by definition and does not have cilksan guarantee finding determinacy races.)

A **data race** occurs when two logically parallel instructions holding no locks in common access the same memory location and at least one of the instruction is a write. Data-race-free programs obey atomicity constraints, but since acquiring a lock is nondeterministic, they still have determinacy race.

No data races is not equal to not having bugs. E.g.
```
slot = hash(x->key);
// lock
x->next = table[slot].head;
// unlock
// lock
table[slot].head = x;
// unlock
```

**Benign races**. E.g. example the set of digits in an array. (sequence of setting and resetting bitfields don't matter, unless in code it looks like setting bytes but architectures actually fetches word, masks a set and writes word back.). Some argue no determinacy races are benign.

##### Implementing mutex

Properties:
* yielding / spinning.
* re-entrant / non-re-entrant. A re-entrant lock allows thread holding it to reenter. Non-re-entrant (faster) deadlocks in this scenario.
* fair / unfair. A fair mutex puts waiting threads on a FIFO queue and unblocks whoever waited longest. Unfair mutex can unblock anyone (starvation).

Simple **spinning mutex** using atomic `xchg` in assembly. `try_get; xchg; test_acquire`. The first part does not help with correctness but helps with performance, because `xchg` instruction involves a write, which requires MSI protocol to broadcast an invalidation, two processors would incur a broadcast storm trying to acquire the lock.

Simple **yielding mutex** in assembly. Replace `pause` (there for unknown intel implementation reasons) in `try_get` with a `yield`.

**Competitive mutex**.
Competing goals: you want to acquire the lock as soon as it's free (spin), but you don't want to busy spin too much (yielding).
Considering processor context switching frequency: 60/s or 100/s, you could spin for as long as a context switch takes (about 10ms, on the order of disk access), then yield.
You never wait longer than twice the optimal time in this strategy. (If the mutex is released while spinning, optimal; if the mutex is released while yielding, `<=2x` optimal)
Ski rental problem: renting until equal to the price of buying.

A clever randomized algorithm can achieve a competitive ratio of `e / e^-1 = 1.58`.

##### Deadlock

Holding more than one lock at a time can be dangerous.
Conditions for deadlock: mutual exclusion, nonpreemption, circular waiting.
Remove any of these constraint can remove deadlock.
Dining philosophers.
Live locks possibility of yielding one when cannot get the other (and the requirement of a random delay).

Assume that we can linearly order the mutexes, then always grab `L_i` before attempting to hold `L_j` for `i < j` breaks the potential deadlock. (_does this echo with 2PL somewhat?_)

Example: deadlocking Cilk with one lock (for Cilk, only hold mutexes within strands).

##### Transactional memory

Example: concurrent Gaussian elimination (in matrix and graph)

Specify in code `atomic` like you do `begin transaction; ...; end` and expect your memory to behave atomically.

Conflict, contention resolution, forward progress, throughput.

Algorithm L
* Finite ownership array: an array of queueing mutex which supports acquire, try_acquire, release
  * owner function maps the memory space U to indexes in the lock (say, with a hash)
  * to lock location x in memory, acquire `lock[h(x)]` where `h` is the owner function
* Release sort reacquire
  * Before accessing a memory location `x`, try to acquire `lock[h(x)]` greedily. On conflict, roll back the transaction without releasing locks, release all locks with indexes higher than `h[x]`, acquire `lock[h(x)]`, blocking if already held. Require the released blocks in sorted order, blocking if already held, then restart transaction. (_only holding locks that are smaller, and each time I restart I lock one larger lock?_)

##### Convoying

### Synchronization without locks

##### Memory models

Sequential consistency:
* the sequence of instructions as defined by a processor's program are interleaved with corresponding sequences defined by the other processors' programs to produce a global linear order of all instructions.
* A load instruction receives the value stored to the address by the most recent store instruction that precedes the load, according to the linear order.

Imagine initially `a = b = 0`. Then
Processor 0:
```s
mov 1, a     # store
mov b, %ebx  # load
```
Processor 1:
```s
mov 1, b     # store
mov a, %eax  # load
```
Is it possible for `%eax` and `%ebx` both to have the values 0, after the above are executed in parallel?

With sequential consistency, no.
But no modern machines implement sequential consistency.

##### Mutual exclusion without locks

(Or special instructions like test-and-set, compare-and-swap, xchg, load-linked-store-conditional)

Can mutual exclusion be implemented with load and store as the only memory operations?
It can, as long as the system is sequentially consistent.

**Peterson's algorithm**.
```cpp
Widget x; // protected variable
bool a_wants = false;
bool b_wants = false;
char turn = 0;

// processor A
{
    a_wants = true;
    turn = 'b';
    while (b_wants && turn == 'b') {};
    // critical section
    x.doSomething();
    // end critical section

    a_wants = false;
}

// processor B
{
    b_wants = true;
    turn = 'a';
    while (a_wants && turn == 'a') {};
    // critical section
    x.doSomething();
    // end critical section

    b_wants = false;
}

```
Happens before and proof by contradiction.
(intuition: the last turn setter waits; works if only one wants; starvation freedom.)
Peterson's algorithm cannot be applied to more than 2 parallel code paths.


##### Relaxed memory consistency

And why some say "never synchronize through memory".

Hardware and compilers actively reorder instructions.

Consider
```s
mov 1, a     # store
mov b, %ebx  # load
```
Why might you want to order a load before a store?
Because load you'll have to wait for it to finish before using the result, not for store.
Hence higher performance by covering load latency - instruction level parallelism.
This reordering matters not when there is no concurrency (as long as `a != b`).

In terms of how this reordering happens in hardware:
There is a queue (per-processor store buffer) between processor and memory system: buffers writes.
Load bypass takes priority over store buffer. This can be problematic when you are loading something in the store buffer, and the load checks store buffer first.

x86 has its own total store order.
* Loads not reordered with loads.
* Stores not reordered with stores.
* Stores not reordered with prior loads.
* Load may be reordered before a prior store to a different location.
* Store, lock respect a global total order.
* Memory ordering preserves transitive visibility (causality).

Instruction reordering violates sequential consistency.
Load-before-store reordering can break Peterson's algorithm, too.

**Memory barrier** (memory fence) is a hardware action that enforces an ordering constraint between the instructions before and after the fence. (Typical cost like an L2 cache access).
Lock instructions implicitly has memory fence.
With memory fence `atomic_thread_fence()` (and to make sure compiler doesn't reorder or optimize your variables to be stored only in registers, use compiler fence and volatile variables; alternatively, C11 `atomic_load` and `atomic_store`) Peterson's algorithm achieves critical section when relaxed memory consistency.

Theorem: any n-thread deadlock-free mutual exclusion algorithms using only load and store requires `Omega(n)` space.
On modern hardware, you also have to use memory fence or atomic compare-and-swap.

##### compare-and-swap

`cmpxchg` asm, `atomic_compare_exchange_strong` C function.
```cpp
bool cas(T *x, T old, T new) {  // atomic, implicit fence
    if (*x == old) {
        *x = new;
        return true;
    } else {
        return false;
    }
}

// then lock, unlock becomes
void lock(int *lock_var) {
    while (!cas(lock_var, false, true));
}

void unlock(int *lock_var) {
    *lock_var = false;
}
```

Lock free algorithms.
Example, when `cas` can outperform `lock`.

### Domain specific language and autotuning

DSL, higher level of abstraction.

Graph algorithm:
* topology-driven algorithms, applied on the entire graph; e.g. building a recommendation system
* data-driven algorithms, starting and ending at certain nodes; e.g. shortest path
Push traversal, pull traversal. Partitioning for parallelism, locality.

**Optimization tradeoff space**: locality, parallelism, work-efficiency.

GraphIt decouples algorithm (what) from optimization (how) for graph applications.

Halide image processing DSL and compiler, also decouples algorithm from scheduler / optimization. Used in e.g. Youtube and ios Photoshop. Trials and errors.

* OpenTuner
Tuning: model based, heuristics based (e.g. if number of elements < 16, use insertion sort, otherwise use parallel quicksort), exhaustive search, autotuning (define the space of acceptable values, choose a value at random, evaluate performance, if performance doesn't satisfy requirement, choose another value and try again. Hillclimb.)

### Speculative parallelism

Guess you can do stuff in parallel, occasionally things will go wrong.

Example: thresholding a sum: is the sum of an unsigned integer array larger than a threshold or not?
Ideas for optimization
* Check and early break (predictable branch short circuiting)
* Unrolling loop and one check per several
* Parallelizing: divide and conquer sim. With an abort flag (whose `set` results in a benign race) to short circuit (why check abort flag before setting it)

**Speculative parallelism** occurs when a program spawns some parallel work that might not be performed in a serial execution.
**Rule of thumb**: don't spawn speculative work unless there is no other opportunity for parallelism and there is a good chance the speculative work will end up being needed.

##### Game Tree Search

**Min-Max Search**.
Two players. Game tree represents all moves within a given search **ply** (depth).
Max chooses the max-scoring one among its children, and Min chooses min scoring one among its children.
**Alpha-Beta strategy**. `[alpha, beta]` range.
For a game tree with branching `b` and depth `d`, an alpha-beta search with moves searched in best-first order examines exactly `b^[floor(d/2)] + b^[ceiling(d/2)] - 1` nodes at ply `d`.

**Principal variation search** pruning. Scout search, improve pruning over alpha-beta.
Alpha-beta and principal variation search depend on putting the best moves at the front to trigger an early cut-off (best-first order).

[Optimizations](https://www.chessprogramming.org/): transposition table. (`map[board_status] = score`). Zobrist hashing can be updated incrementally. Killer-move table (chess killer heuristic). Best-move table. Null-move pruning. Futility pruning. Late move reduction. Opening book. Iterative deepening (for move ordering information).

##### Parallel Alpha-Beta Strategy

Young siblings wait algorithm.

### Tuning a TSP algorithm

Recursive generation.
* Iterative bit counting solution (+1). 
* Recursive solution (divide into last bit 0 and last bit 1)

Algorithm 1: `n!` all permutations (swap) recursion

External optimizations:
* `gcc -O3`: x25 faster vs no optimization
* Faster hardware: x150 faster compared with 20 yrs ago

Internal optimizations:
* distance table lookup as opposed to dist calc on the fly. 3x.
* Fix a start, Nx.
* Carry forward a partial sum as opposed to recomputing sum. `(1 + e)`x

Exponential growth: `n! ~ 2^(n lg(n)) ~ (n / e) ^ n`

Pruning search space:

Example. Given 1..9, find all permutation such that first m digits are divisble by m.
* Pruning: even in even digits, odd in odd digits, 5 in position 5. 600x search space reduction.

Pruning TSP:
* if sum already larger than current known minsum, don't continue. >100x speedup.
* if sum + lower bound for remaining cities > minsum, don't continue. The lower bound can be minimum spanning tree of remaining. >100x speedup.
* cache lazy-evaluated MST. >10x speedup.

In performance engineering, your intuition is often times wrong. Experiment.

Smarter search:
* better starting tour: start with a greedy nearest neighbor

Presumably, state of the art TSP does the above optimizations better, and can solve >10k nodes optimally.

### Graph optimization

Graph representations:
* adjacency matrix (`O(|V|^2)`),
* edge list (`O(|E|)`),
* adjacency list (`O(|V| + |E|)`),
* Compressed sparse row: offsets array, edges array (`O(|V| + |E|)`). To store weights, have an additional array, or preferably, have the weights interleaved,

Complexity of adding / removing an edge, finding all neighbors, finding if a vertex w is a neighbor of v.

CSR is not a good choice when needing to update the graph, but good for static graph algorithms.

Properties of real world graphs (twitter, web, search),
* billions of edges and vertices, ~500GB for search, ~10GB for Twitter.
* sparse
* degrees are skewed (power law degree distribution) (implies load imbalance issues if just parallelize by vertices)

##### BFS

Serial algorithm. Work: `O(|V| + |E|)`. Cache misses analysis. Bit vector visited to improve cache performance.

Parallel algorithm. Parallelize over nodes in the frontier. Beware of races (non-determinism with `compare-and-swap`) and load balancing. One frontier needs to be finished iterating, before moving on to the next frontier. Span analysis.

Direction optimization for BFS.

Frontier representation.

Ligra graph framework.

##### Graph compression and ordering

Further compression on top of CSR. Variable length encoding.

### High performance in dynamic languages

Fast fourier transformation, FFTW.

Julia.
* High level, interactive, general-purpose, for technical work but as fast as C.
* Most of Julia is implemented in Julia.
* Dynamically typed using inference + specialization. Abstract types (hierarchy).
* Multiple dispatch (all the argument types (e.g. `method(object, x, y)`) decide the method, as opposed to single dispatch as `object.method(x, y)`.)
* Type instability (ideally, return type should be known at compile time. `sqrt` always return `double`, even if on a perfect square. This means `sqrt(-1)` returns an error, instead of a complex value, as doing the latter would mean type is determined at runtime. This instability propagates, and eventually the language might end up relying boxes and runtime type tags, and this hurts performance.)
* Parametric polymorphism (similar to C++ templates with constraint that the supplied type, e.g. has to be a subtype of something)
* All concrete types are final (otherwise the size cannot be known at compile time. Python does not enforce this, hence it's not possible to implement NumPy in Python.)
* Macros and meta programming
* Parallel facilities

Python integer never overflows, this means `x + 1` cannot be compiled into one instruction, this could also hurt performance.

Python numba compiler looks at arguments, type specializes, calls llvm, and compiles the function into llvm code. Only works primitive types (native and numpy, not arbitrary user numeric types).

Implementation of Python lists: pointers to boxes (of a type tag and a value), this can't be as fast a C loop over array of doubles.
NumPy array does this.
NumPy array sum uses SIMD instructions, and will be faster than naive C for loop sum.
Python `sum` will for faster than loop and `operator+` (due to caching `operator+`?)

PyPy, tracing JIT for python with more implemented in Python as opposed to C.





Do sequentially consistent in memory model and causal consistent in distributed system mean the same thing?

`__restrict` keyword can give the compiler more freedom to do optimizations, knowing this is the only pointer pointing to the data.

Signed and unsigned shift (_what does signed shift do?_)

(Log time Fibonacci number algorithm)

(_why does 3 layers of cache imply 12 loops when doing tiling?_)

(_why does the recursive version look like that in slides?_)