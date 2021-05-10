# [Writing good C++14](https://www.youtube.com/watch?v=1OEu9C51K2A)

Coding guidelines: many poor ones out there, limited to particular domains, not easily enforceable, some are limited to platform / compiler.

Tell what to do / what not to do with a rationale.

Never tell people what not to do without an alternative.

GSL core guidelines.

Core rules, no leaks (RAII), no dangling pointers, no type violation through pointers (both possibly enforced by the type system).

* Use RAII to manage lifetime
* Having ownership checked at runtime (with an extra bit) is possible, but possibly expensive and does not address all issues.
* Having ownership as mental abstraction and making it explicit in type system is the way to go. (`gsl::owner<T>` as a low level abstraction in the type system. `delete` cannot happen from non-owner, non-owner and owner cannot be mixed in a `vector`, etc)
* Dealing with range errors: `gsl::array_view<T>`: abstract over `(ptr, cnt)`.
* `gsl::not_null<T>`: enforcing `not_null` from type system, as opposed to documentation, or runtime checks.

Similar concepts, `absl::span`, `std::reference_wrapper`, etc.

Smart pointer usage
* often overused when you don't need to deal with ownership.
* when you are not transferring / sharing ownership, do not pass unique or shared ptr by value.

Ideally, the tools know the rules.


```cpp
extern int x = 10; // extern is ignored.
```
A declaration with an initializer is always definition.

A variable defined without an initializer in the global or a namespace scope is initialized by default.
This is not the case for non-static local variable or objects created on the free store (heap / dynamic storage).

External linkage: a name that can be used in translation units different from the one in which it was defined.
Internal linkage: name that can only be referred to within the translation unit in which it is defined.
No linkage: names that linker does not see e.g. local variables.

`static` in namespace scope means internal linkage.
`const` (or `constexpr`, `using` / type aliases) in namespace scope implies internal linkage, to let it have external linkage, add `extern` to `const`.
`inline` function in every translation unit must be declared identically.



