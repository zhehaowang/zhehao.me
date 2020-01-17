### Understand the reasons for `ptr_fun`, `mem_fun` and `mem_fun_ref`

What do they do anyway?

If I have a function `f` and an object `x`, I want to invoke `f` on `x` and I'm outside `x`'s member functions, I can do
```
f(x);   // when f is non-member function
x.f();  // when f is member function and x is an object or reference to an
        // object
p->f(); // when f is member function and p is a pointer to x
```

Now suppose I have this
```
void test(Widget& w); // test w and mark it as failed if it doesn't pass
vector<Widget> vw;

// I can do for_each
for_each(vw.begin(), vw.end(), test);

// now imagine `test` is a member function of Widget instead
struct Widget { test(); ... };

// this won't compile
for_each(vw.begin(), vw.end(), &Widget::test);

// also won't compile
vector<Widget*> pw;
for_each(pw.begin(), pw.end(), &Widget::test);
```

The later two calls need two different forms of invoking `test` on `widget`.

In reality `for_each` invokes its template function `f` in the non-member form `f(x)`.

Hence we have `mem_fun` and `mem_fun_ref` to adapt the invocation to different forms.

They are function templates with several variants corresponding to different numbers of parameters and constness.

E.g. `mem_fun` is declared like this
```
template <typename R, typename C>
mem_fun_t<R, C> mem_fun(R (C::*pmf)());  // C is the class, R is the return type
```
This takes a pointer to a member function `pmf` and returns `mem_fun_t`. This is a functor class that holds the member function pointer and offers an `operator()` that invokes the pointed-to member function on the object passed to `operator()`.

`mem_fun` adapts syntax 3 to syntax 1, `mem_fun_ref` adapts syntax 2 to 1.

The objects produced by `mem_fun` and `mem_fun_ref` also provide typedefs like `ptr_fun` mentioned in item 40.

When giving STL algorithm a member function, for your code to compile you are forced to use `mem_fun` / `mem_fun_ref`.

The names are a bit weird as `mem_fun` adapts the container of pointers `->call` syntax and `mem_fun_ref` adapts the `.call` syntax.
