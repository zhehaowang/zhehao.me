# Pythonic thinking

### Use list comprehensions instead of `map` and `filter`

List comprehension is a compact syntax for deriving one list from another.

Unless you are applying a single-argument function, list comprehensions are clearer than the `map` built-in function for simple cases. `map` requires creating a lambda which is visually noisy.

```
a = [1, 2, 3]
squares = [x ** 2 for x in a]
squares = map(lambda x: x ** 2, a)
```

Unlike `map`, list comprehensions let you easily filter items from the input list, removing corresponding outputs from the result.

```
even_squares = [x ** 2 for x in a if x % 2 == 0]
```

`filter` can be used along with `map` to achieve the same outcome, but is much harder to read

```
even_squares = map(lambda x: x ** 2, filter(lambda x: x % 2 == 0, a))
```

Dictionaries and sets have their own equivalents of list comprehensions.

```
a = {'a': 1, 'b': 2}
b = {v: k for k, v in a.items()}      # b = {1: 'a', 2: 'b'}
a_set = {len(a) for a in b.values()}  # a_set = {1, 1}
```

**Takeaways**
* List comprehensions are clearer than `map` and `filter` built-in functions as they don't require extra lambda expressions.
* List comprehensions allow you to easily skip items from the input list, a behavior `map` can't support without `filter`.
* Dictionary and set also support comprehension expressions.
