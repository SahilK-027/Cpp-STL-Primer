# Lesson 7: unordered_map and unordered_set

`unordered_map` and `unordered_set` are **hash tables**: the same jobs as `map` and `set` ([Lesson 6](./Lesson_6_Set_Map.md)), but with **no order**, in exchange for **O(1) average** insert, erase and lookup instead of O(log n).

```cpp
unordered_map<string, int> cnt;
cnt["apple"]++;                 // same API as map

unordered_set<int> seen;
seen.insert(5);
if (seen.count(5)) { ... }      // O(1) on average
```

This lesson answers **five questions**:

1. [**How** does a hash table work (just enough to reason about it)?](#how-does-a-hash-table-work)
2. [**How** is it different from `map` and `set`?](#how-is-it-different-from-map-and-set)
3. [**How** do you use `pair` or struct keys?](#how-do-you-use-pair-or-struct-keys)
4. [**Why** can it TLE in contests? (CP-only.)](#why-can-it-tle-in-contests-cp-only)
5. [**How** do you make it faster, and when do iterators break?](#how-do-you-make-it-faster-and-when-do-iterators-break)

It ends with the [rules that always apply](#rules-that-always-apply).

---

## How does a hash table work?

We'll build one up from scratch, one idea at a time. Each new word is introduced only after the idea behind it.

### Step 1: compute where a key goes

The goal is to find a key **without searching through everything**. The idea: keep an array, and use a formula on the key to decide which slot of the array it lives in. To find the key later, run the same formula and look in that one slot.

With an array of 4 slots and integer keys, a simple formula is `key % 4`:

```
slot of 6  = 6 % 4 = 2
slot of 3  = 3 % 4 = 3

slot:    0      1      2      3
                       [6]    [3]
```

To check whether 6 is present: compute `6 % 4 = 2`, look in slot 2. One step, no matter how many keys there are.

### Step 2: two keys can want the same slot

`1 % 4 = 1` and `5 % 4 = 1`, so 1 and 5 both want slot 1. When two keys want the same slot, that's called a **collision**. The fix: each slot holds a small list of keys instead of just one. Each slot, together with its little list, is called a **bucket**.

```
bucket:  0      1            2      3
                [1] [5]      [6]    [3]
```

Looking up 5 now means: go to bucket 1, then check the keys in its list until you find 5.

### Step 3: turning any key into a number

`% 4` only works on numbers. To store a string like `"apple"`, first turn it into a number. The function that does that is the **hash function**, and its result is the key's **hash**. The bucket is then `hash % (number of buckets)`.

For integers, GCC's hash function just returns the number itself; for strings it mixes all the characters into one number.

### Step 4: why it's O(1) on average, and O(n) at worst

A lookup costs "jump to the bucket" (one step) plus "check that bucket's list". If keys spread out evenly, every list stays tiny, so a lookup is **O(1) on average**. If many keys land in the same bucket, that one list gets long, and a lookup has to check up to all n keys: **O(n) worst case**.

### Step 5: growing the table (rehashing)

As you keep inserting into a fixed number of buckets, the lists get longer. The table tracks the average list length, `number of keys / number of buckets`, which is called the **load factor**.

When the load factor passes a limit (1.0 by default), the table builds a bigger array, about twice as many buckets. But each key's bucket was `hash % (number of buckets)`, and that number just changed, so every key has to be moved to its new bucket:

```
with 4 buckets:   1 % 4 = 1    5 % 4 = 1    6 % 4 = 2    9 % 4 = 1

  bucket:  0     1              2     3
                 [1] [5] [9]    [6]              bucket 1 is crowded

with 8 buckets:   1 % 8 = 1    5 % 8 = 5    6 % 8 = 6    9 % 8 = 1

  bucket:  0   1          2   3   4   5     6     7
               [1] [9]                [5]   [6]
```

Moving every key into a bigger table is called a **rehash**. It costs O(n), but it happens rarely (only when the table doubles), so inserts are still O(1) on average, just like `vector::push_back` growing its capacity ([Lesson 5](./Lesson_5_Vector_String.md#what-is-the-difference-between-size-and-capacity)).

### Step 6: what a rehash does to iterators and references

Think of a library moving its books onto twice as many shelves.

- **The books themselves aren't copied.** The same physical books are just placed on different shelves. So a reference or pointer to an element still points to the same element: **still valid**.
- **Positions change.** "The 2nd book on shelf 1" is now a different book, or nothing. An iterator is a position in the table's walk order, and that order has changed, so old iterators are **invalid**.

A vector is different: when it grows, it copies its elements into new memory, so even references break.

---

## How is it different from `map` and `set`?

### What stays the same

Insert, look up and erase work exactly as in [Lesson 6](./Lesson_6_Set_Map.md#how-do-you-insert-and-look-up), including the traps:

```cpp
unordered_map<string, int> mp;
mp["a"] = 1;                    // insert or overwrite
mp.count("b");                  // check without inserting
mp.find("b") != mp.end();       // same, with the iterator
mp.try_emplace("a", 5);         // insert only if absent
mp.erase("a");

for (const auto& [key, val] : mp) { ... }
```

> [!WARNING]
> **`mp[key]` still inserts a missing key**, exactly as in `map`. Use `count` or `find` to check.

`unordered_multiset` and `unordered_multimap` also exist, with the same "erase by value removes every copy" rule.

### What's missing: everything that needs order

| Feature | `map` / `set` | `unordered_map` / `unordered_set` |
|---|---|---|
| insert, find, erase | O(log n) | O(1) average, O(n) worst |
| iteration order | sorted | arbitrary, and can change after inserts |
| `lower_bound`, `upper_bound` | yes | no |
| smallest / largest (`begin`, `rbegin`) | yes | no (there's no `rbegin` at all) |
| key needs | `<` | a hash and `==` |
| `pair`, `vector` as key | works | needs a custom hash ([next section](#how-do-you-use-pair-or-struct-keys)) |

If you need neighbours, min/max or sorted output, use `map` / `set`. If you only need "is it there?" or counts, the unordered versions are faster.

> [!NOTE]
> **Don't rely on the iteration order.** It depends on the hash values and the bucket count, so it can differ between compilers and even change in the middle of your program after a rehash. If the output must be sorted, copy the keys into a vector and sort them, or use `map`.

---

## How do you use `pair` or struct keys?

C++ provides hashes for numbers, `char` and `string`, but **not** for `pair`, `tuple`, `vector` or your own structs. So this fails to compile:

```cpp
unordered_map<pair<int,int>, int> mp;     // error: no hash for pair<int,int>
```

There are three ways around it, from simplest to most general.

### Option 1: just use `map`

`map<pair<int,int>, int>` works with no extra code, because `map` only needs `<`, and pairs have it ([Lesson 4](./Lesson_4_Pair_Tuple.md#how-do-they-compare)). It costs O(log n) instead of O(1), which is fast enough for most problems. **Start here.**

### Option 2: encode the pair into one number

Pack two ints into one 64-bit number, and use a plain `unordered_map<unsigned long long, int>`:

```cpp
auto key = [](int a, int b) {
    return ((unsigned long long)(unsigned int)a << 32) | (unsigned int)b;   // a high, b low
};
unordered_map<unsigned long long, int> mp;
mp[key(r, c)] = 5;
```

Every different pair gives a different number, so nothing collides by accident. For grid cells you can also use `r * m + c`, where `m` is the number of columns.

### Option 3: write a custom hash

A hash for `unordered_*` is a **functor** ([Lesson 3's "other forms"](./Lesson_3_Comparators.md#what-other-forms-will-you-see)): a struct with `operator()` that turns a key into a `size_t`. It goes in the **3rd template slot**:

```cpp
struct PairHash {
    size_t operator()(const pair<int,int>& p) const {
        unsigned long long k = ((unsigned long long)(unsigned int)p.first << 32) | (unsigned int)p.second;
        return hash<unsigned long long>()(k);
    }
};
unordered_map<pair<int,int>, int, PairHash> mp;
unordered_set<pair<int,int>, PairHash> st;
```

> [!TIP]
> **Prefer lambdas?** [The next section](#writing-a-hash-with-a-lambda-for-every-key-type) writes every hash as a lambda instead of a struct.

**Struct keys need two things**: a hash (as above) and `operator==`, so the table can tell two keys in the same bucket apart:

```cpp
struct Point {
    int x, y;
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
};
struct PointHash {
    size_t operator()(const Point& p) const {
        unsigned long long k = ((unsigned long long)(unsigned int)p.x << 32) | (unsigned int)p.y;
        return hash<unsigned long long>()(k);
    }
};
unordered_set<Point, PointHash> pts;
```

| Option | Code needed | Cost | Use when |
|---|---|---|---|
| `map` / `set` | none | O(log n) | default; fine for most problems |
| encode into `long long` | one line | O(1) avg | pairs of ints, grid cells |
| custom hash | a struct | O(1) avg | structs, or when encoding gets awkward |

### Writing a hash with a lambda, for every key type

A hash is any function that turns a key into a number (a `size_t`), giving **equal keys equal numbers** and spreading different keys out. So it can be a lambda, used just like a lambda comparator ([Lesson 3](./Lesson_3_Comparators.md#where-does-the-comparator-go)), with one extra detail in the declaration:

```cpp
auto h = [](const Key& k) -> size_t { ... };
unordered_map<Key, Value, decltype(h)> mp(0, h);
unordered_set<Key, decltype(h)> st(0, h);
```

The `0` is there because the constructor's first parameter is the starting number of buckets, and you must pass something (`0` lets the table choose) before you can pass `h`. From C++20 you can write just `mp;` for lambdas with no captures.

#### Which keys need a hash at all?

| Key type | Hash needed? | How |
|---|---|---|
| `int`, `long long`, `char`, `string` | no: built in | nothing to write |
| `pair<int,int>` | yes | pack both into one 64-bit number |
| a `pair` of anything else, `tuple` | yes | combine the parts' hashes |
| `vector<int>` | yes | combine every element's hash |
| your own struct | yes | combine the fields' hashes, and give the struct `operator==` |

#### `pair<int,int>`: pack it into one number

An `int` is 32 bits and a `long long` is 64, so one `long long` has room for two ints side by side. It's like combining 12 and 34 into 1234: shift the first number left to make room, then put the second in the gap.

```cpp
auto h = [](const pair<int,int>& p) -> size_t {
    unsigned long long k = ((unsigned long long)(unsigned int)p.first << 32)   // first: top 32 bits
                         | (unsigned int)p.second;                              // second: bottom 32 bits
    return hash<unsigned long long>()(k);
};
unordered_map<pair<int,int>, int, decltype(h)> mp(0, h);
```

- `<< 32` moves `first` into the top half (the same as multiplying by 2³²), and `|` drops `second` into the empty bottom half.
- Every different pair gives a different number, so there are no accidental collisions.
- The `unsigned` casts keep negative numbers from spilling into the other half.

#### Everything else: combine the parts' hashes

When the parts don't fit side by side in 64 bits (`pair<long long,long long>`, `pair<string,int>`, vectors, structs), hash each part with the built-in `hash<T>`, then **combine** the results. This is the standard combining step (from the Boost library); copy it as is:

```cpp
auto combine = [](size_t seed, size_t v) {
    return seed ^ (v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
};
```

> [!WARNING]
> **Don't combine with plain `^`.** `hash(a) ^ hash(b)` gives the same result for `{1, 2}` and `{2, 1}`, and gives 0 for every `{x, x}`, so many different keys collide. The extra shifts and the constant in `combine` mix the bits so that order matters.

**A pair of other types:**

```cpp
auto h = [&](const pair<string,int>& p) -> size_t {
    return combine(hash<string>()(p.first), hash<int>()(p.second));
};
unordered_map<pair<string,int>, int, decltype(h)> mp(0, h);
```

**A vector:** fold `combine` over every element. Note that hashing a vector reads all its elements, so each lookup costs O(length of the vector), not O(1).

```cpp
auto h = [&](const vector<int>& v) -> size_t {
    size_t seed = v.size();
    for (int x : v) seed = combine(seed, hash<int>()(x));
    return seed;
};
unordered_set<vector<int>, decltype(h)> seen(0, h);
```

**A struct:** combine the fields, and give the struct `operator==` so the table can tell two keys in the same bucket apart:

```cpp
struct Point {
    int x, y;
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
};
auto h = [&](const Point& p) -> size_t {
    return combine(hash<int>()(p.x), hash<int>()(p.y));
};
unordered_set<Point, decltype(h)> pts(0, h);
```

`h` captures `combine` with `[&]`, so define `combine` first, in the same scope.

> [!TIP]
> **What to reach for first.** `map` / `set` needs no hash at all. Next simplest is encoding a pair into one `long long` ([Option 2](#option-2-encode-the-pair-into-one-number)). Write a hash lambda only when you truly need O(1) with a composite key.

---

## Why can it TLE in contests? (CP-only)

In GCC, the built-in hash for integers is the number itself, and the bucket counts follow a fixed, publicly known list of primes. Someone who knows this can build a test where **every key lands in the same bucket**. Then every operation checks all n keys, so a solution you expected to be O(n) becomes **O(n²)** and times out.

Codeforces problems often include such **anti-hash tests**, and other contestants can submit them as hacks. Interviewers don't do this, so in interviews you only need to know that the worst case is O(n) and why.

### The fix: a randomised hash

Scramble each key with a strong mixing function, plus a random seed chosen when the program starts, so nobody can predict which bucket a key lands in:

```cpp
struct custom_hash {
    static unsigned long long splitmix64(unsigned long long x) {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }
    size_t operator()(unsigned long long x) const {
        static const unsigned long long SEED =
            chrono::steady_clock::now().time_since_epoch().count();
        return splitmix64(x + SEED);
    }
};

unordered_map<long long, int, custom_hash> mp;   // safe against anti-hash tests
unordered_set<long long, custom_hash> st;
```

You don't need to understand the constants: `splitmix64` is a well-known mixer, and the seed changes on every run. Keep this struct in your contest template and paste it in when you use `unordered_*` with integer keys. For pair keys, encode the pair into one number ([Option 2](#option-2-encode-the-pair-into-one-number)) and hash that with `custom_hash`.

> [!TIP]
> **Or just use `map`.** If O(log n) fits the time limit, `map` is immune to anti-hash tests and needs no extra code.

---

## How do you make it faster, and when do iterators break?

### `reserve`

Every rehash re-places all keys. If you know roughly how many keys you'll insert, allocate enough buckets up front so no rehash happens:

```cpp
unordered_map<long long, int> mp;
mp.reserve(n);              // room for n keys without rehashing
```

In contests you'll also see a lower load factor, which means more buckets and fewer collisions at the cost of memory. Set it **before** `reserve`, because `reserve` uses it to decide how many buckets to allocate:

```cpp
mp.max_load_factor(0.25);
mp.reserve(1 << 20);
```

### When iterators break

A rehash moves keys into new buckets, so iterators stop being valid. But each element stays at the same address in memory, so references and pointers to elements survive ([Step 6](#step-6-what-a-rehash-does-to-iterators-and-references)).

| Operation | Iterators | References and pointers to elements |
|---|---|---|
| insert that triggers a rehash | **all invalid** | still valid |
| insert without a rehash | still valid | still valid |
| `erase` | only the erased elements | only the erased elements |

> [!WARNING]
> **Don't insert while iterating over an `unordered_map`.** A rehash in the middle of the loop invalidates the loop's iterator. Collect the new keys in a vector first, then insert them after the loop.

The three containers so far, side by side:

| Container | Insert can invalidate iterators? | Insert can invalidate references? |
|---|---|---|
| `vector` | yes, all of them, on reallocation | yes |
| `map` / `set` | no | no |
| `unordered_map` / `unordered_set` | yes, all of them, on rehash | no |

---

## Rules that always apply

1. O(1) average, O(n) worst case. No order, so no `lower_bound`, no min/max, and no stable iteration order.
2. `mp[key]` inserts a missing key, exactly like `map`. Check with `count` or `find`.
3. Keys need a hash and `==`. Numbers, `char` and `string` have both; `pair`, `tuple`, `vector` and structs need a custom hash (or use `map`, or encode the pair into one `long long`).
4. A custom hash is a functor in the 3rd template slot: `unordered_map<K, V, Hash>`.
5. Struct keys also need `operator==`.
6. (CP-only) Against anti-hash tests, use the randomised `custom_hash`, or `map`.
7. `reserve(n)` before inserting about n keys avoids rehashes; set `max_load_factor` before `reserve`.
8. A rehash invalidates all iterators, but not references or pointers. Don't insert while iterating.

---

<p align="center">
  <a href="./Lesson_6_Set_Map.md">← Previous: Lesson 6, set, multiset, map, multimap</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a> &nbsp;·&nbsp; <a href="./Lesson_8_Priority_Queue.md">Next: Lesson 8, priority_queue and heaps →</a>
</p>
