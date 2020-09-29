# PyCon 2017 dictionary

Instances have dictionaries all behind them.

Different Python versions (2.7, 3.5, 3.6) differ in dictionary size and ordering.
3.6 and on features ordered dictionary keys.

```py
class C:
    def __init__(self, v1, v2):
        self.k1 = v1
        self.k2 = v2
        self.k3 = 'x'

o1 = C(1, 2)
o2 = C(3, 4)
o3 = C(5, 6)
```
An object of this class is backed by a dictionary.
In py3.6, its keys are ordered and deterministic (`k1` comes first, then `k2`).

To store `o1`, `o2`, `o3` we can do **list of tuples**
```py
[('k1', 1, 3, 5),
 ('k2', 2, 4, 6),
 ('k3', 'x', 'x', 'x')]
```
Or we can store **association lists**
```py
[
    [('k1', 1), ('k2', 2), ('k3', 'x')],
    [('k1', 3), ('k2', 4), ('k3', 'x')],
    [('k1', 5), ('k2', 6), ('k3', 'x')],
]
```
Or **separate chaining** (reducing the search space with hashtable)
```py
[
    [
        [('k1', 1), ('k3', 'x')],
        [('k2', 2)]
    ],
    [
        [('k1', 3), ('k3', 'x')],
        [('k2', 4)]
    ],
    [
        [('k1', 5), ('k3', 'x')],
        [('k2', 6)]
    ]
]
```
**Dynamic resizing**: resize the dictionary to never be two thirds full.

**Caching the full hash value (of a key, 64 bits space)** and store it inside the hashtable to avoid rehashing.
(Otherwise when we resize we need to find out the hashed value again to decide which new bucket it goes to.)
(Two hashes involved here, one is the full hash of a key, the other putting the full hash into buckets)

**Faster matching**: not use `__eq__` of two keys (potentially slow, say key is some arbitrary tuple), but rather compare the two hashes first
```py
def fast_match(key, target_key):
    if key is target_key:
        # fast, identiy check
        return True
    if key.hash != target_key.hash:
        # fast, hash check
        return False
    # potentially slow.
    # A dictionary lookup does exactly one equality test
    # even if there are collisions, (which is rare in
    # practice).
    return key == target_key
```

(Note that IEEE 754 says `float.NaN != float.NaN`.)

**Open addressing / probing**: make the table more dense.
Go to the hashed slot, if it's not this one, go to the next slot (linear).
This as-is makes it difficult to delete keys, and we have tombstones. (Knuth - algorithm D)

Problem now is linear lookup pile-up: we want the hashed keys to end up in fairly distant slots.
We do **multiple hashing**, e.g. `i = (5 * i + perturb + 1) % number_of_buckets` when we run into a collision, where `perturb` is initialized with the full 64bit hash, and then shifted by 5 bits each time `perturb = perturb >> 5`, this tries to make sure we use the full hash in dealing with collision, and also does not get in a loop.

Say for `o1`, we now have
```py
[
    (f231b30932e6208a, 'k3', 'x'),
    None,
    (de321098434b1401, 'k1', 1),
    None,
    None,
    None,
    (32014bea31234124, 'k2', 2),
    None,
]
```

(A dictionary has a **`private` version number**, a dictionary, when not mutated, keeps the same version number such that the same lookups can be sped up.)

**Compact dict** lets us not keep the full size for `None` entries:
```py
# o1
# array of values
[
    (de321098434b1401, 'k1', 1),
    (32014bea31234124, 'k2', 2),
    (f231b30932e6208a, 'k3', 'x'),
],

# index (8B each item)
[2, None, 0, None, None, None, 1, None]

# ...
# same thing for o2 and o3
```

The above as-is sees the array of values of repeated 3 times for the 3 objects `o1`, `o2` and `o3`, while the hash values are the same, just that values for each dict key is different for the 3 objects.
To address this we have **key sharing dictionary**, which looks like the following to store the 3 objects:

```py
# array of values
[
    (de321098434b1401, 'k1', 1, 3, 5),
    (32014bea31234124, 'k2', 2, 4, 6),
    (f231b30932e6208a, 'k3', 'x', 'x', 'x'),
],

# index (1B each item, 8B total)
[2, None, 0, None, None, None, 1, None]
```

So really, a list of tuples indexed by a dictionary.

The future has **sparse index table**, such that collision is rare.

The full hash is cryptographic and reseeded each time Python starts up.
Sets use multiple chaining and linear probing.
Cuckoo hashing can be used, but the current design doesn't look like a win (Cuckoo is meant to be dense, current direction is going for sparse).
We have SipHash in Python for strings to avoid deliberate collisions.
Internally dicts and sets guard against mutation while iterating.
