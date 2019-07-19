# Be on alert for most vexing parse

Suppose you have a file of ints and you'd like to copy those ints into a list.
This seems reasonable.

```
ifstream datafile("ints.dat");
list<int> data(istream_iterator<int>(datafile), istream_iterator<int>());
```

But not really.
It'll compile but not do anything at runtime.

Start with the basics
```
int f(double d);   // declares a function
int f(double (d)); // declares the same function, parens around d are ignored
int f(double);     // declares the same function, param name is omitted

int g(double (*pf)()); // g takes a pointer to a function
int g(double pf());    // same, pf is implicitly a pointer
int g(double ());      // same, parameter name is omitted
```

Notice that if `()` wraps a parameter name, they are ignored. If they appear standing by themselves, they indicate a parameter that is itself a pointer to a function.

And our example at the start actually declares a function: first parameter is `datafile` and parentheses are ignored. Second parameter has no name. Its type is pointer to function taking nothing and returning an `istream_iterator<int>`.

This is a universal rule in C++: anything that can be parsed as a function declaration will be.
Like this
```
class Widget {  ...  }; // assumes default ctor
Widget w();             // declares a function. does not define an object.
```

Knowing our problem, we can add parentheses to enclose our function call.
```
list<int> data((istream_iterator<int>(datafile)), istream_iterator<int>());
```

This is the proper way to declare data, but not every compiler implements this correctly: they would instead accept the wrong way.
Modern g++, clang should yield a warning in this case, and propose where to add parentheses.

Or, to avoid this, do the following instead:
```
ifstream datafile("ints.dat");
istream_iterator<int> databegin(datafile);
istream_iterator<int> dataend;
list<int> data(databegin, dataend);
```
This use of named iterator objects runs contrary to common STL programming style, but it may be worth it as the code is unambiguous to compilers and the humans working with them.

**Takeaways**
* Beware of most vexing parse parsing anything that can pass as a function declaration as a function declaration. To avoid this when actually making variable declaration, try enclosing parentheses around a parameter.
