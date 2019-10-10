# Item 48. Always `#include` proper headers

Software that compiles on one platform may require additional `#include` directives on others.

This stems from the fact that the standard for C++ fails to dictate which standard headers must or may be `#include`d by other standard headers, and different implementers have chosen to do different things.

`<vector>` / `<algorithm>` may or may not include `<string>`, `<iostream>` may or may not include `<iterator>`, `<string>` may or may not include `<algorithm>`, and `<set>` may or may not include `<functional>`.

You are required to include proper standard headers for your code.

To make things easier,
* almost all containers are declared in headers of the same name. `vector` in `<vector>`, etc. One exception is `multiset` is in `<set>` and `multimap` is `<map>`.
* all but four algorithms are declared in `<algorithm>`, exceptions are `accumulate`, `inner_product`, `adjacent_difference` and `partial_sum` are declared in `<numeric>`.
* special kinds of iterators like `istreambuf_iterator`s are declared in `<iterator>`.
* standard functors like `less<T>` and functor adapters `not1`, `bind2nd` are declared in `<functional>`.

Any time you use these components be sure to include the appropriate headers, even though your development platform may let you get away without it.
