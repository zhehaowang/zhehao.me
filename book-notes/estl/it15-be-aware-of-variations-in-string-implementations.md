# Be aware of string implementation variations

There are many string implementations.

What is `sizeof(string)`? It depends on the underlying implementation.

Virtually every string implementation holds the following information: size of the string (number of characters), capacity of the string, value of the string, and optionally, a copy of its allocator.
String implementation that uses reference counting may also have a reference count.

One implementation A may define string as
```
+-----------+
| allocator |
+-----------+
|   size    |
+-----------+
|  capacity |
+-----------+       +---------+-------------+
|   p_data  | ----> | ref_cnt |   value ... |
+-----------+       +---------+-------------+
```
And `sizeof(string)` is four times that of a pointer.

Another implementation B may have it as
```
+-----------+
|   p_impl  |
+-----------+
      |
      |
+-----------+
| allocator |
+-----------+
|   size    |
+-----------+
|  capacity |
+-----------+       +---------+-------------+
|   p_data  | ----> | ref_cnt |   value ... |
+-----------+       +---------+-------------+
| others e. |
| g. concur-|
| rency ctrl|
+-----------+
```

And another implementation C may be
```
+-----------+
|   p_impl  |
+-----------+
      |
      |     +-----------+
      |     |   size    |
      |     +-----------+
      |     |  capacity |
      |     +-----------+ 
      |     |   p_data  | 
      |     +-----------+ 
      |     |     x     |
      +---> +-----------+
            |   value   |
            |    ...    |
            +-----------+
```
Where `x` has to do with the shareability of the objects (ME C++ it 29).

And another one D with small string optimization, where `sizeof(string)` can be 7 times that of a pointer.
```
+-----------+              +-----------+
| allocator |              | allocator |
+-----------+              +-----------+
|   value   |              |   p_data  | ----> +-----------+
|    ...    |              |  (unused) |       | value ... |
+-----------+              +-----------+       +-----------+
|   size    |              |   size    |
+-----------+              +-----------+
|  capacity |              |  capacity |
+-----------+              +-----------+

(when size <= 15)          (when size > 15)
```

This means `string s("abc");` will be zero dynamic allocation under D, one under A and C, and two under B.

In a design based on reference counting, everything outside the string object can be shared by multiple strings if they have the same value.
A offers less sharing B and C as A doesn't share the string's size and capacity.
C does not support per-object allocator means all strings must use the same allocator.
D does not use reference counting.

They may also have different capacity to store our small string: A has a minmum allocation of 32 characters where 31 can be used to store chars and the last one is null.
B has no minimum, the capacity is the size.
C has 16 as minimum and no trailing null.
D also minimum buffer size 16.

Let's summarize things that can vary:
* string values may or may not be reference counted. By default many implementations offer it with a way to turn off. Reference counting helps when strings are frequently copied, and some applications just don't copy strings often enough to justify the overload.
* string objects size may vary from one pointer to seven times size of a pointer.
* creation of a new string may require 0, 1, or 2 dynamic allocations.
* string objects may or may not share information on the string's size and capacity.
* strings may or may not support per-object allocators.
* different implementations have different policies regarding minimum allocations for character buffers.

Know your string implementation, especially if you write code that runs on different STL platforms and you face stringent performance requirements.
