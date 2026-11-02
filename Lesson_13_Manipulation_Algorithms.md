# Lesson 13: manipulation algorithms

The `<algorithm>` header has a set of small tools that **rearrange, clean up and summarise** ranges. Each one replaces a loop you'd otherwise write by hand, and a few have traps worth knowing.

```cpp
vector<int> v = {3, 1, 3, 2, 1};
sort(v.begin(), v.end());                          // 1 1 2 3 3
v.erase(unique(v.begin(), v.end()), v.end());      // 1 2 3: duplicates removed
reverse(v.begin(), v.end());                       // 3 2 1
cout << min({4, 9, 2});                            // 2: the min of several values
```

This lesson answers **six questions**:

1. [**How** do you remove duplicates?](#how-do-you-remove-duplicates)
2. [**How** do you reorder elements?](#how-do-you-reorder-elements)
3. [**How** do you go through every permutation?](#how-do-you-go-through-every-permutation)
4. [**How** do you find a min and max?](#how-do-you-find-a-min-and-max)
5. [**How** do you fill and count?](#how-do-you-fill-and-count)
6. [**How** does it all come together? (Coordinate compression, a worked example)](#worked-example-coordinate-compression)

It ends with the [rules that always apply](#rules-that-always-apply).

---

## How do you remove duplicates?

### Step 1: duplicates are easy to spot when they're neighbours

In a **sorted** vector, equal values sit next to each other: `1 1 2 3 3`. So removing duplicates only needs one pass: keep an element if it differs from the one before it.

### Step 2: `unique` does that pass

`unique(b, e)` walks the range and packs the elements that differ from their left neighbour to the front. It works exactly like `remove_if` from [Lesson 5](./Lesson_5_Vector_String.md#erasing): **nothing is deleted**, the leftovers stay at the end, and it returns where the leftovers start.

```
sorted:         1  1  2  3  3
after unique:   1  2  3  ?  ?
                         ^ returned iterator: the new logical end
```

### Step 3: erase the leftovers

Cut them off with `erase`, using the same two-argument pattern as erase-remove:

```cpp
sort(v.begin(), v.end());
v.erase(unique(v.begin(), v.end()), v.end());     // v now holds each value once, sorted
```

> [!WARNING]
> **`unique` only removes ADJACENT duplicates.** On `{3, 1, 3}`, nothing is removed, because the two 3s aren't neighbours. Always sort first, unless you specifically want to collapse runs (e.g. turning `"aaabbbaa"` into `"aba"`).

**Counting distinct values without erasing:** after sorting, `unique(v.begin(), v.end()) - v.begin()` is the number of distinct values.

**Alternative:** `set<int> s(v.begin(), v.end());` also removes duplicates, in one line. It's often slower in practice (a tree of nodes rather than one array), but either is fine for most problems.

---

## How do you reorder elements?

### reverse

```cpp
reverse(v.begin(), v.end());                      // whole vector
reverse(v.begin() + l, v.begin() + r + 1);        // only indices l..r (half-open range, Lesson 1)
reverse(s.begin(), s.end());                      // strings too
```

O(n). The `r + 1` is there because ranges are [half-open](./Lesson_1_Iterators.md#half-open-ranges-begin-end). A common interview use: checking a palindrome with `string t = s; reverse(t.begin(), t.end());` then `t == s`.

### rotate

`rotate(first, middle, last)` shifts the range so that `middle` becomes the first element, with everything before it wrapping around to the end:

```cpp
vector<int> v = {1, 2, 3, 4, 5};
rotate(v.begin(), v.begin() + 2, v.end());    // 3 4 5 1 2: rotated left by 2
```

To rotate **left** by `k`, the new first element is `v.begin() + k`. To rotate **right** by `k`, the last `k` elements should come first, so the new first element is `v.end() - k`:

```cpp
k %= n;                                        // rotating by n changes nothing
rotate(v.begin(), v.begin() + k, v.end());     // left by k
rotate(v.begin(), v.end() - k, v.end());       // right by k
```

> [!WARNING]
> **Reduce `k` first.** If `k` can be larger than `n`, `v.begin() + k` points past the end, which is undefined behaviour. `k %= n` fixes it (as long as `n > 0`).

### swap

```cpp
swap(a, b);                  // any two variables of the same type
swap(v[i], v[j]);            // two elements
swap(v1, v2);                // two whole vectors: O(1), it just swaps their internal pointers
```

### shuffle (mostly CP)

To put a range in random order (for randomised algorithms or test data), use `shuffle` with a properly seeded random generator:

```cpp
mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
shuffle(v.begin(), v.end(), rng);
```

The older `random_shuffle` was removed in C++17.

---

## How do you go through every permutation?

### Step 1: put all the orderings in dictionary order

The orderings of `1 2 3` can be listed the way a dictionary lists words: compare the first element, then the second, and so on.

```
1 2 3  →  1 3 2  →  2 1 3  →  2 3 1  →  3 1 2  →  3 2 1
```

Each ordering is a **permutation**, and this order is called **lexicographic**. The first one is fully ascending, and the last one fully descending.

### Step 2: step to the next one

`next_permutation(b, e)` rearranges the range into the next permutation in that list and returns `true`. When it's already at the last one, it wraps around to the first (ascending) and returns `false`.

### Step 3: visit them all

Sort first, so you start at the beginning of the list, and use a `do { } while` loop, so the starting arrangement is processed before the first step:

```cpp
vector<int> v = {3, 1, 2};
sort(v.begin(), v.end());                          // start at 1 2 3
do {
    // use v: 1 2 3, then 1 3 2, 2 1 3, 2 3 1, 3 1 2, 3 2 1
} while (next_permutation(v.begin(), v.end()));
```

> [!WARNING]
> **Forgetting to sort skips orderings.** Starting from `3 1 2`, the loop only visits `3 1 2` and `3 2 1`, because those are the ones after it in the list.

- **With duplicates**, each distinct arrangement appears exactly once: `{1, 1, 2}` gives 3 arrangements, not 6.
- **Cost:** n elements have n! orderings (10! ≈ 3.6 million), and each step is O(n). So this brute force works up to about **n = 10**.
- `prev_permutation` goes the other way, and both work on strings: `string s = "abc";` with `next_permutation(s.begin(), s.end())`.

> [!NOTE]
> **"Next Permutation" is a classic interview question** (LeetCode 31). `next_permutation` solves it in one line, but the interviewer usually wants you to explain or implement the algorithm behind it, so treat the library call as the answer to check against.

---

## How do you find a min and max?

### Of a few values

```cpp
min(a, b);                 max(a, b);
min({a, b, c, d});         max({a, b, c, d});   // any number of values: put them in braces
clamp(x, lo, hi);          // C++17: x forced into [lo, hi], same as max(lo, min(x, hi))
auto [lo, hi] = minmax(a, b);                    // both at once, as a pair
```

> [!WARNING]
> **Both arguments must have the same type.** `min` doesn't convert between types, so mixing them is a compile error:
>
> ```cpp
> int k; long long x;
> min(k, x);                    // error: int vs long long
> min<long long>(k, x);         // fix: say which type to use
> min(k, (int)v.size());        // v.size() is unsigned (Lesson 5): cast it
> ```

### Of a whole range

These return **iterators**, so you get the position as well as the value:

```cpp
auto it = max_element(v.begin(), v.end());
int value = *it;
int index = it - v.begin();                        // the FIRST occurrence of the max

auto [mnIt, mxIt] = minmax_element(v.begin(), v.end());   // both in one pass
```

- **On an empty range** they return `end()`, and `*it` is undefined behaviour, so check first.
- `min_element` and `max_element` return the **first** occurrence. `minmax_element` returns the first minimum but the **last** maximum, a rarely needed detail that matters if you use the index.
- All of them take a comparator, which is still "less than" even for max ([Lesson 3](./Lesson_3_Comparators.md#min_element-and-max_element)).

---

## How do you fill and count?

### fill and count

```cpp
fill(v.begin(), v.end(), -1);               // every element = -1
fill(v.begin() + l, v.begin() + r + 1, 0);  // only indices l..r
v.assign(n, 0);                             // resize to n AND fill (Lesson 5)

count(v.begin(), v.end(), 3);               // how many 3s: O(n)
count(s.begin(), s.end(), 'a');             // how many 'a's in a string
count_if(v.begin(), v.end(), [](int x) { return x < 0; });   // how many negatives (Lesson 2)
```

### memset: why it only works for a few values

**Step 1:** `memset(ptr, value, bytes)` comes from C. It doesn't know about `int`s; it sets **every byte** in a block of memory to `value`.

**Step 2:** an `int` is made of 4 bytes. So `memset` with value `v` makes every `int` equal to the number whose 4 bytes are all `v`:

| memset value | Each byte | Each int becomes | Useful? |
|---|---|---|---|
| `0` | `00` | 0 | Yes |
| `-1` | `FF` | -1 | Yes |
| `0x3f` | `3F` | `0x3f3f3f3f` = 1,061,109,567 | Yes: a common "infinity" for `int`, and doubling it still fits |
| `1` | `01` | `0x01010101` = 16,843,009 | **No**: not 1 |

```cpp
int dist[100];
memset(dist, 0, sizeof dist);       // all 0: fine
memset(dist, 1, sizeof dist);       // NOT all 1: every element is 16843009
```

> [!WARNING]
> **Don't `memset` a vector.** `sizeof v` is the size of the vector object itself (a few pointers), not of its elements. For vectors, use `fill` or `assign`, which also work for any value.

**The rule:** use `memset` only on plain arrays, and only with `0`, `-1` or `0x3f`. For everything else, `fill`.

---

## Worked example: coordinate compression

### The problem

You have up to 2 × 10⁵ values, each as large as 10⁹. You'd like an array **indexed by value** (to count occurrences, or for a structure like a Fenwick tree), but an array of size 10⁹ is far too big. Often only the **relative order** of the values matters, not the values themselves.

### The idea

Replace each value by its **rank** among the distinct values: the smallest becomes 0, the next 1, and so on. Ranks are always below n, so they fit in a small array, and they keep the order: `a[i] < a[j]` exactly when `rank[i] < rank[j]`. Squeezing values down to their ranks like this is called **coordinate compression**.

### The steps, from three lessons

1. **Copy** the values.
2. **Sort** them and **remove duplicates**: `sort` ([Lesson 10](./Lesson_10_Sorting.md)), then `unique` + `erase` ([this lesson](#how-do-you-remove-duplicates)). Now `vals` lists each distinct value once, in order, so a value's position in `vals` is its rank.
3. For each original value, **find its position** with `lower_bound` ([Lesson 11](./Lesson_11_Binary_Search.md#what-does-each-function-return)).

```cpp
vector<int> a = {100, 5, 100, 42};

vector<int> vals = a;                                          // 1. copy
sort(vals.begin(), vals.end());                                // 2. sort:    5 42 100 100
vals.erase(unique(vals.begin(), vals.end()), vals.end());      //    dedupe:  5 42 100

vector<int> rank(a.size());
for (int i = 0; i < (int)a.size(); i++)
    rank[i] = lower_bound(vals.begin(), vals.end(), a[i]) - vals.begin();   // 3. position = rank

// rank = {2, 0, 2, 1}
```

| `a[i]` | 100 | 5 | 100 | 42 |
|---|---|---|---|---|
| **position in `vals` = rank** | 2 | 0 | 2 | 1 |

**Cost:** O(n log n): one sort, then one binary search per value. To get an original value back from its rank, read `vals[rank]`.

> [!NOTE]
> **A `map` version works too:** insert every value into a `map<int, int>`, then number the keys in order. It's also O(n log n) but slower in practice; the sort + `unique` + `lower_bound` version is the standard one.

---

## Rules that always apply

1. Remove duplicates: `sort`, then `v.erase(unique(v.begin(), v.end()), v.end());`. `unique` only removes **adjacent** duplicates.
2. `unique`, like `remove`, deletes nothing by itself: it returns where the leftovers start, and `erase` cuts them off.
3. `rotate(b, b + k, e)` rotates left by k; `rotate(b, e - k, e)` rotates right by k. Reduce `k %= n` first.
4. All permutations: sort first, then `do { ... } while (next_permutation(b, e));`. Feasible up to about n = 10.
5. `min` / `max` need both arguments of the same type: `min<long long>(a, b)`, or cast. `min({a, b, c})` for several values.
6. `min_element` / `max_element` return iterators (first occurrence). Check for an empty range before `*`.
7. `memset` sets **bytes**: only `0`, `-1` and `0x3f` give sensible `int`s, and never use it on a vector. Otherwise use `fill`.
8. Coordinate compression: copy, sort, `unique` + `erase`, then `lower_bound(...) - vals.begin()` is each value's rank.

---

<p align="center">
  <a href="./Lesson_12_Numeric_Overflow.md">← Previous: Lesson 12, numeric tools and overflow</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a>
</p>
