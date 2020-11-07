# [Refactoring at scale](https://www.youtube.com/watch?v=ZpvvmvITOrk)

Cutting the Gordian Knot. At scale.

Can't do sed regex, by IDE or hand.

Given sufficient use, there is no such thing as a private implementation.
Somebody somewhere is going to depend on the implementation detail.

Hope is not a strategy.

Solution: tooling based off of compiler AST.
Tool: ClangMR (Clang + MapReduce) (ClangMR is single translation unit)
Input source (monolithic codebase) --> Clang AST --> custom matcher / edit --> MapReduce --> edit decision list --> modified code

clang-tidy, clang-modernizer also use similar infrastructure.

Assumes continuous testing system, clang-format, (auto) code review (for auto change), sharded into smaller changes (which are independent) and test independently (cannot submit in one patch, conflicts and what if rollbacks)
