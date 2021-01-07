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
