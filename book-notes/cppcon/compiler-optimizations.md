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

LLVM IR (intermediate representation, distilled form of client program) (similarities between modern optimizers are much bigger than differences) is meant to be simple.

Instructions
LLVM IR control flow, blocks, labeled jumps
LLVM IR data flow, single static assignment (SSA form, gcc / intel / ibm also uses this; SSA makes const propagation trivial; with SSA local data flow becomes straightforward); after something's defined, they cannot be reassigned / redefined; Phi node: where the data flew from (to reconcile / merge data defined in two branching blocks)

Optimizer does not just make your code fast.
* It also needs to **clean up** generated code from frontend. Frontend usually assumes unlimited memory and uses loads and stores freely. LLVM turns that into smart SSA labels
* **Canonicalization**. You can write the same thing with a ternary expression or an if-else. LLVM IR canonicalize different ways of writing into the same representation; so that the optimizer handles fewer patterns. This is what LLVM is really good at.
* **Collapse abstractions**. Three key abstractions: functions, calls, call graph; memory, loads and stores; loops
  * inlining is the single most important optimization in modern compilers. But don't confuse this with C++ keyword inline. Two different things.
    * simplifying call graph: given a call graph, first partition into strongly connected components. For each component, LLVM does bottom-up SCC based walk: start with an SCC without outgoing edges, inline the calls in there, consider this SCC as optimized, and then move up to another SCC (like what we do in a topological sort). Within one SCC since it's cyclic, there isn't much you can do other than trying starting with different nodes. gcc inliner does top-down. In reality both LLVM and GCC do a hybrid approach.
    * collapsing wrappers: func a returns b(different order of arguments); `fancy_sort` for sort with special handling for 1 and 2 elements. Example: variadic template recursion with one less argument each time (tail recursion hash, e.g.)
  * memory, pointer arithmetic, partitioning an object into isolated memory blocks. Memory optimizer boils down to finding some way to isolate a particular location in memory and doing a partial job of forming the SSA values (and track them through the control flow).




