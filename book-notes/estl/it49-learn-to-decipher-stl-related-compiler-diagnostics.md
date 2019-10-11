# Item 49. Learn to decipher STL-related compiler diagnostics

`string` errors, `string` is a `typedef` for this `basic_string<char, char_traits<char>, allocator<char> >`.

This is because C++ notion of a `string` has been generalized to mean sequences of arbitrary character types with arbitrary characteristics (traits) and stored in memory by arbitrary allocators.

All string-like objects are really instantiations of the template `basic_string`.

When dealing with such compiler messages with internal types (like `_Tree`) and substituted templates (like the `basic_string<char, char_traits<char>, allocator<char> >` above, or `std::map<class string, class string, struct std::less<class string>, class std::allocator<class string> >`), consider a text replacement.

A few other hints:
* `vector` and `string` iterators are usually pointer types, and diagnosing `vector<double>::iterator` your compiler may bring up `double*`.
* messages mentioning `back_insert_iterator`, `front_insert_iterator` or `insert_iterator` usually indicates you've a mistake calling `back_inserter`, `front_inserter` or `inserter`, whose corresponding return types are the former.
* `binder1st` and `binder2nd` are probably `bind1st` `bind2nd` usage errors
* output iterators like `ostream_iterator`) do their outputting or inserting work inside assignment operators, if you get errors in an assignment operator using these
* if you get some error within an STL algorithm, it's probably the wrong types you are trying to use with that algorithm, e.g. passing the wrong type of iterators (like giving bidirectional when requiring random access ones)
* check header inclusion if your compiler does not recognize the likes of `vector`
