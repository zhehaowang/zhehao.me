# Prefer `iterator` to `const_iterator`, `reverse_iterator` and `const_reverse_iterator`

The suggestion may be deprecated.

C++11 changed `vector::insert`, `vector::erase` to take in `const_iterator`.
And because there is implicit conversion from `iterator` to `const_iterator` but not the other way round, using `const_iterator` with the old standards can be quite limiting in terms of what member functions you can use.

Also some old STL implementation declares `const_iterator::operator==` as a member function as opposed to a free friend function. This makes equality comparison between `iterator` and `const_iterator` not compile, unless you write the `iterator` first.

The conclusion given an older standard was that sometimes `const_iterator` was useful but just not worth the trouble. This also appears no longer true with the new standard.
