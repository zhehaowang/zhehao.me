### Item 46. Consider function objects instead of functions as algorithm parameters

It's not generally true that as the level of abstraction gets higher, the efficiency of the generated code gets lower.

Passing STL function objects to algorithms typically yields code that is more efficient than passing real functions.

E.g.
```
vector<double> v;
...
sort(v.begin(), v.end(), greater<double>());

// vs

inline bool doubleGreater(double d1, double d2) {
    return d1 > d2;
}
...
sort(v.begin(), v.end(), doubleGreater);
```
You may be surprised to find the version using `greater<double>` is always faster than the passing function version.

The explanation is simple: inlining.
If a function object's `operator()` function has been declared inline (explicitly or implicitly by having it in class definition), the body of that function is available to compilers, which is the case for `greater<double>::operator()`.

In the case with `doubleGreater`, we are passing a pointer to a function as a parameter to another function.
The declaration will look like the following:
```
void sort(vector<double>::iterator first, vector<double>::iterator last, bool (* comp)(double, double));
```
Most compilers won't try to inline calls to functions that are invoked through function pointers, even if the function is declared `inline` in this case.

The fact that function pointer parameters inhibit inlining explains an observation that long-time C programmers often find hard to believe: C++'s `sort` virtually always embarrasses C's `qsort` when it comes to speed, as the latter uses a function pointer that many compilers do not inline.

Another reason to prefer function objects has nothing to do with efficiency, but getting your code to compile.
One STL platform rejects this valid code
```
set<string> s;
...
transform(
    s.begin(),
    s.end(),
    ostream_iterator<string::size_type>(cout, "\n"),
    mem_fun_ref(&string::size));
```
The problem is the particular platform has a bug in handling of const member function such as `string::size`, and a workaround is using an object instead:
```
struct StringSize : public unary_function<string, string::size_type> {
    string::size_type operator()(const string& s) const {
        return s.size();
    }
};

transform(s.begin(),
          s.end(),
          ostream_iterator<string::size_type>(cout, "\n"),
          StringSize());
```
This sidesteps the compilation problem, and is also likely to improve efficiency as this facilitates inlining the call to `string::size`, something `mem_fun_ref(&string::size)` usually does not do.

Another reason to prefer function objects to functions is that they can help you avoid subtle language pitfalls: occasionally source code that looks reasonable is rejected by compilers for legitimate but obscure reasons.
E.g. when the name of an instantiation of a function template is not equivalent to the name of a function.
```
template <typename FPType>
FPType average(FPType val1, FPType val2) {
    return (val1 + val2) / 2;
}

// write the average of the two input ranges to the given stream
template <typename InputIter1, typename InputIter2>
void writeAverages(InputIter1 begin1,
                   InputIter1 end1,
                   InputIter2 begin2,
                   ostream& s) {
    transform(
        begin1,
        end1,
        begin2,
        ostream_iterator<typename iterator_traits<InputIter1>::value_type>(
            s, "\n"),
        average<typename iterator_traits<InputIter1>::value_type>);
}
```
Many compilers accept this code, but the standard appears to reject it, reason being there could be another function template named `average` that takes a single type parameter, as if so the expression `average<typename iterator_traits<InputIter1>::value_type>` would be ambiguous.

In this case we could also fall back on a function object:
```
template <typename FPType>
struct Average : public binary_function<FPType, FPType, FPType> {
    FPType operator()(FPType val1, FPType val2) const {
        return average(val1, val2);
    }
};

template <typename InputIter1, InputIter2>
void writeAverages(InputIter1 begin1, InputIter1 end1, InputIter2 begin2, ostream& s) {
    transform(
        begin1,
        end1,
        begin2,
        ostream_iterator<typename iterator_traits<InputIter1>::value_type>(
            s, "\n"),
        Average<typename iterator_traits<InputIter1>::value_type>());
}
```

This should be accepted by every compiler, plus call to `Average::operator()` inside `transform` is eligible for inlining, not the case in our previous `average` template instantiation.

Function objects as parameters to algorithms thus offer more than greater efficiency: they are also more robust when it comes to getting your code to compile.
