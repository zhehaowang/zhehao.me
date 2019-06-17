### Compilers Optimization

* Why C++: performance
* C++ does not make your code fast magically, it gives you tremendous amount of control
* It also gives you really powerful compiler optimizations. It may feel like magic, but it really isn't

Goal of this talk
* Understand how to work efficiently with a compiler, and how your code is optimized.
* Understanding performance means understanding what compilers do.

Language Frontend --> Optimization --> Code Generation

Modern compiler design focuses more on optimization, as opposed to supporting language frontend.
(Dragon Book: Compilers Principles, Techniques and Tools, does not talk at all about optimization)

LLVM IR (intermediate representation, distilled form of client program) is meant to be simple. (similarities between modern optimizers are much bigger than differences)

##### Instructions

LLVM IR control flow, blocks, labeled jumps
LLVM IR data flow, single static assignment (SSA form, gcc / intel / ibm also uses this; SSA makes const propagation trivial; with SSA local data flow becomes straightforward); after something's defined, they cannot be reassigned / redefined; Phi node: where the data flew from (to reconcile / merge data defined in two branching blocks)

Optimizer does not just make your code fast.
* It also needs to **clean up** generated code from frontend. Frontend usually assumes unlimited memory and uses loads and stores freely. LLVM turns that into smart SSA labels
* **Canonicalization**. You can write the same thing with a ternary expression or an if-else. LLVM IR canonicalize different ways of writing into the same representation; so that the optimizer handles fewer patterns. This is what LLVM is really good at.
* **Collapse abstractions**. Three key abstractions: functions, calls, call graph; memory, loads and stores; loops
  * inlining is the single most important optimization in modern compilers. But don't confuse this with C++ keyword inline. Two different things.
    * simplifying call graph: given a call graph, first partition into strongly connected components. For each component, LLVM does bottom-up SCC based walk: start with an SCC without outgoing edges, inline the calls in there, consider this SCC as optimized, and then move up to another SCC (like what we do in a topological sort). Within one SCC since it's cyclic, there isn't much you can do other than trying starting with different nodes. gcc inliner does top-down. In reality both LLVM and GCC do a hybrid approach.
    * collapsing wrappers: func a calls and returns b(different order of arguments); `fancy_sort` for sort with special handling for 1 and 2 elements. Example: variadic template recursion with one less argument each time (tail recursion hash, e.g.)
  * memory, pointer arithmetic, partitioning an object into isolated memory blocks (breaking a structure into individual variables, and promote the individual variables to registers). Memory optimizer boils down to finding some way to isolate a particular location in memory (with partitioning being the easiest and the most predominent way) and doing a partial job of forming the SSA values (and track them through the control flow).
  * loop optimization
    * (side note C++ tmp linear algebra library eigen)
    * LLVM loop canonical form. (preheader, loop and exit blocks)
    * Unrolling loop into straight line code (if trip count is fixed, replace the loop with sequentially executed instructions)
    * Move code out of loops, e.g. loading a variable in a loop but that variable does not change during the loop (loop invariant in a phi node; e.g. calling vector.size() inside a loop. You don't want programmers remembering to cache every such in local variables)
    * Split the two when you have an if-else in a loop but the if-condition does not change during loop
    * vectorize things to make the most out of your processor: widen and interleave the loops to leverage CPU pipeline. (if you have at least two iterations in your loop, do two iterations at the same time. Keep a scalar fallback of the origin loop to handle odd bits). Vectorization / software pipelining
    * phase ordering matters: in what order do we carry out these optimizations matter. (some transformations are information preserving, e.g. unrolling, while others may lose information (e.g. the structure of a loop))
    * some compilers infer your loop invariants from your asserts, in the future aliasing, annotations might be standardized to let you give hints to compilers about optimizing your loop (invariants, likely / unlikely)

LLVM models atomic operations directly from C++ memory model. If generated code says non-atomic load, then LLVM knows we don't have the concern of multiple threads reading / writing this memory location at the same time.

Know the trade-offs you are making when, e.g. manually unrolling / vectorizing your loop, job of a compiler is to let you write your code with the abstraction you are comfortable with.

The compiler frontend does not do optimizations, except taking out dead code. Separation of concerns.

When the abstractions are combined, it's where the compiler struggles the most.
At any point you can mix fewer abstractions, consider doing so. E.g. calling a wrapper function with pass an integral value by reference. Compiler needs to optimize memory and inlining.
Consider using return by value in this case instead.

const qualified methods or const ref parameters have no utility for compiler optimization whatsoever. Both can be const_casted away.

People often undervalue the cost of pinning something in memory, when considering passing by reference vs passing by value.

Understand what compilers optimize, and work with the compiler.

(JVM can do runtime optimization, profile the program as you run it, identify hotspots, and let jit makes a smarter decision the next time this code is ran. This is easier to do with a GC.)

Sanitizers insert checks for bugs when you are compiling. UBSan e.g. checks for undefined behaviors.
If you suspect your code is breaking because of a bug in compiler optimization, run your code through a sanitizer. If the sanitizer doesn't catch anything from your code, then it's either the sanitizer or the compiler's fault. Compiler does not optimize for it, if a sanitizer does not sanitize for it.
