### Use `accumulate` or `for_each` to summarize ranges

`count` `count_if` `min_element` `max_element` can all summarize a range into a single number.
But if you need to summarize a range in some custom manner, try `accumulate` in `<numeric>`.

`accumulate` exists in two forms:
```
list<double> ld;
// put some numbers in
double sum = accumulate(ld.begin(), ld.end(), 0.0);
// calculate sum, starting at 0.0
// note the 0.0 is important, had it been written as 0, internally accumulate
// will an int to store the value it's computing. This will compile and run.
```

`accumulate` only requires `InputIterator`s, so you could use `istream_iterator`s and `istreambuf_iterators`:
```
cout << "The sum of the ints on the standard inpt is "
     << accumulate(istream_iterator<int>(cin), istream_iterator<int>(), 0);
```

`accumulate` can also work with an initial value and an arbitrary summarization function.
```
string::size_type
stringLengthSum(string::size_type sumSoFar, const string& s) {
    return sumSoFar + s.size();
}

string::size_type lengthSum = accumulate(
    ss.begin(), ss.end(), 0, stringLengthSum);
```

Every STL container has a `size_type` that is the container's type for counting things. This is the type returned by `size()` call.
For all standard containers, `size_type` must be `size_t`, non-standard containers can have a different `size_type`.

Calculating the product of a range is easier:
```
vector<float> vf;
// fill some numbers in
float product = accumulate(vf.begin(), vf.end(), 1.0, multiplies<float>);
```

And another example
```
struct Point {
    Point(double _x, double _y) : x(_x), y(_y) {}
    double x,y;
};

class PointAverage : public binary_function<Point, Point, Point> {
  public:
    PointAverage() : xSum(0), ySum(0), numPoints(0) {}
    const Point operator()(const Point& avgSoFar, const Point& p) {
        ++numPoints;
        xSum += p.x;
        ySum += p.y;
        return Point(xSum / numPoints, ySum / numPoints);
    }
  private:
    size_t numPoints;
    double xSum;
    double ySum;
};

list<Point> lp;

// fill with some values

Point avg = accumulate(lp.begin(), lp.end(), Point(0, 0), PointAverage());
// remember not to take the initial point into account in the functor class
```

The standard forbids side effect in the function passed to accumulate, so technically speaking the above code yields undefined results.
In practice it's hard to imagine it not working but in writing this may not work.

This brings us to `for_each`, which also takes a range and a function but the function passed to `for_each` receives only a single argument (the current range argument), and `for_each` returns a copy of its function when it's done.
Significantly, the function passed to `for_each` may have side effects.

`for_each` is geared more towards applying a function on each element.
Using it to summarize a range works, but not as clear as `accumulate`. Using `for_each` means we also need to extract the summary information from the returned function object.
The last example then becomes
```
class PointAverage : public unary_function<Point, void> {
  public:
    PointAverage() : xSum(0), ySum(0), numPoints(0) {}
    void operator()(const Point& p) {
        ++numPoints;
        xSum += p.x;
        ySum += p.y;
    }

    Point result() const {
        return Point(xSum / numPoints, ySum / numPoints);
    }
  private:
    size_t numPoints;
    double xSum;
    double ySum;
};

Point avg = for_each(lp.begin(), lp.end(), PointAverage()).result();
```

Why the standard requires `accumulate` function object to be side effect free but not `for_each`'s function object?
That we've yet to hear a convincing explanation.
