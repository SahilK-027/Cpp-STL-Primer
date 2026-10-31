# Lesson 11: the binary search family

Binary search finds things in a sorted range in **O(log n)** steps instead of O(n). The STL gives you a small family of functions for it, and they are all built on **one idea**.

```cpp
vector<int> v = {1, 3, 3, 3, 5, 8};      // must be sorted

lower_bound(v.begin(), v.end(), 3) - v.begin();   // 1: first element >= 3
upper_bound(v.begin(), v.end(), 3) - v.begin();   // 4: first element >  3
binary_search(v.begin(), v.end(), 5);             // true: 5 is present
```

This lesson answers **four questions**:

1. [**How** does binary search work?](#how-does-binary-search-work)
2. [**What** does each function return?](#what-does-each-function-return)
3. [**How** do you find neighbours and count values in a range?](#how-do-you-find-neighbours-and-count-values-in-a-range)
4. [**What** if there's no sorted array, only a yes/no condition?](#what-if-theres-no-sorted-array-only-a-yesno-condition)

It ends with the [rules that always apply](#rules-that-always-apply).

---

## How does binary search work?

### Step 1: the guessing game

I think of a number from 1 to 100; you guess, and I say "higher" or "lower". The best strategy is to **guess the middle** every time: each answer throws away half of what's left. 100 → 50 → 25 → … → 1 takes at most **7 guesses**. For n possibilities, that's about log₂ n steps: **O(log n)**.

### Step 2: the same on a sorted array

To find `x` in a sorted array, look at the middle element. If it's smaller than `x`, then everything to its left is smaller too (the array is sorted), so throw that half away. Otherwise, throw away the right half. This only works **because the array is sorted**: on unsorted data, "everything to the left is smaller" isn't true.

### Step 3: what it's really finding is a boundary

Look at a sorted array with a yes/no question asked of every element:

```
index:      0  1  2  3  4  5
v:          1  3  3  3  5  8

"is it >= 3?"   F  T  T  T  T  T     → the first T is at index 1
"is it >  3?"   F  F  F  F  T  T     → the first T is at index 4
```

Because the array is sorted, the answers are always **all F's, then all T's**: once a question becomes true, it stays true. A question that behaves like this is called **monotonic**.

Binary search finds **the first T**: check the middle; if it's T, the boundary is there or to its left; if it's F, the boundary is to its right. Halve, and repeat.

> [!TIP]
> **Every function in this lesson is "find the first T" with a different question.** `lower_bound(x)` asks "is it ≥ x?", `upper_bound(x)` asks "is it > x?". Remember that, and you can rebuild everything else.

> [!NOTE]
> **O(log n) needs random access.** Jumping to the middle is O(1) on a `vector`, `deque` or array. On a `set` or `map`, use their member `lower_bound` ([Lesson 6](./Lesson_6_Set_Map.md#how-do-you-find-neighbours)); the free function would walk step by step, which is O(n).

---

## What does each function return?

All examples use `v = {1, 3, 3, 3, 5, 8}`, with positions given as indices (`it - v.begin()`).

| Function | Question asked | Returns | `x = 3` | `x = 4` | `x = 9` |
|---|---|---|---|---|---|
| `lower_bound(b, e, x)` | is it ≥ x? | iterator to the first T | index 1 | index 4 | `end()` (index 6) |
| `upper_bound(b, e, x)` | is it > x? | iterator to the first T | index 4 | index 4 | `end()` |
| `equal_range(b, e, x)` | both | `pair{lower_bound, upper_bound}` | [1, 4) | [4, 4): empty | [6, 6): empty |
| `binary_search(b, e, x)` | is x present? | `bool` | `true` | `false` | `false` |

When no element answers T, the boundary is past the last element, so you get **`end()`**. Always check before dereferencing.

### Everyday uses

```cpp
// Find x and its position
auto it = lower_bound(v.begin(), v.end(), x);
if (it != v.end() && *it == x) {
    int pos = it - v.begin();               // first occurrence of x
}

// How many times does x appear?
int cnt = upper_bound(v.begin(), v.end(), x) - lower_bound(v.begin(), v.end(), x);

// The same, with equal_range
auto [lo, hi] = equal_range(v.begin(), v.end(), x);
int cnt2 = hi - lo;
```

> [!NOTE]
> **`lower_bound` is also "where would x go?"** It's the first position where `x` could be inserted while keeping the vector sorted. So `v.insert(lower_bound(v.begin(), v.end(), x), x);` inserts in order (in O(n), because of the shifting).

> [!WARNING]
> **`binary_search` only answers yes or no.** It doesn't tell you where. When you need the position, use `lower_bound` and check `*it == x`, as above.

---

## How do you find neighbours and count values in a range?

### The four neighbour queries

"Smallest" queries are a single call. "Largest" queries use one idea: **the largest element that fits is the one just before the first one that's too big**. So ask what "too big" means, find the first such element, and step back with `prev`.

| You want | "Too big" means | Code | Guard before `*` |
|---|---|---|---|
| smallest element ≥ x | — | `lower_bound(x)` | `!= end()` |
| smallest element > x | — | `upper_bound(x)` | `!= end()` |
| largest element ≤ x | > x, the first is `upper_bound` | `prev(upper_bound(x))` | `upper_bound(x) != begin()` |
| largest element < x | ≥ x, the first is `lower_bound` | `prev(lower_bound(x))` | `lower_bound(x) != begin()` |

With `v = {1, 3, 3, 3, 5, 8}` and `x = 3`:

```
index:   0  1  2  3  4  5
v:       1  3  3  3  5  8
            ^        ^
   lower_bound(3)   upper_bound(3)

largest <= 3:  prev(upper_bound) = index 3 → 3
largest <  3:  prev(lower_bound) = index 0 → 1
```

```cpp
auto it = upper_bound(v.begin(), v.end(), x);
if (it != v.begin()) {
    int largestAtMostX = *prev(it);
} else {
    // every element is > x: nothing is <= x
}
```

The guard is needed because if the very first element is already too big, there's nothing before it.

### Counting with subtraction

The distance between two boundaries is a count. On a vector it's O(1) to subtract, so these are all **O(log n)**:

| Count of elements | Code |
|---|---|
| < x | `lower_bound(x) - v.begin()` |
| ≤ x | `upper_bound(x) - v.begin()` |
| > x | `v.end() - upper_bound(x)` |
| equal to x | `upper_bound(x) - lower_bound(x)` |
| in the range [l, r] | `upper_bound(r) - lower_bound(l)` |

(Each `lower_bound(x)` above is short for `lower_bound(v.begin(), v.end(), x)`.)

For `v = {1, 3, 3, 3, 5, 8}`, the count in [2, 5] is `upper_bound(5) - lower_bound(2)` = index 5 − index 1 = **4** (the three 3s and the 5).

> [!NOTE]
> **The same recipes work on a `set` or `map`**, using their member functions (`s.lower_bound(x)`, `prev(s.upper_bound(x))`). Counting by subtraction doesn't: set iterators can't be subtracted, and `distance` walks, so it's O(n).

---

## What if there's no sorted array, only a yes/no condition?

[Step 3](#step-3-what-its-really-finding-is-a-boundary) showed that binary search only needs a **monotonic** question: answers that switch once and never switch back. A sorted array with "≥ x" is just the most common case.

### `partition_point`: binary search with any condition

`partition_point(b, e, pred)` takes a condition that is **true for a prefix, then false** for the rest, and returns the **first element where it's false**.

> [!WARNING]
> **The polarity is flipped.** `partition_point` wants T…T then F…F, the opposite of "find the first T". So phrase the condition as **"is it still too small?"**.

```cpp
vector<int> v = {1, 3, 3, 3, 5, 8};
auto it = partition_point(v.begin(), v.end(), [](int y) { return y < 5; });
// still too small? T T T T F F  →  first F at index 4: the same as lower_bound(5)
```

Its real value is with structs. Searching a vector of jobs sorted by deadline needs no mixed-parameter lambdas ([Lesson 3](./Lesson_3_Comparators.md#searching-structs-by-one-field)):

```cpp
// jobs sorted by deadline: first job with deadline >= 10
auto it = partition_point(jobs.begin(), jobs.end(),
                          [](const Job& j) { return j.deadline < 10; });
```

### Binary search on the answer (an algorithm pattern)

Sometimes the thing you're searching for isn't in any container: it's a **number** such as a minimum speed or a maximum capacity. If the question "does value k work?" is monotonic (it fails for small k and works from some point on), binary search over k itself.

There's no range to pass to an STL function, so you write the loop. The cleanest version gives the two pointers **fixed roles**, so there's never a "`mid + 1` or `mid - 1`?" decision:

- `lo` always sits on a value that **fails** (F) when minimising, and on one that **works** (T) when maximising;
- `hi` always sits on the opposite kind;
- the loop shrinks the gap until they're neighbours, and the boundary lies between them.

**Minimise:** the smallest k that works. `F F F T T T`: find the first T.

```cpp
long long lo = LOW - 1, hi = HIGH;         // lo: surely fails (never tested); hi: surely works (never tested)
while (lo + 1 < hi) {
    long long mid = lo + (hi - lo) / 2;    // always strictly between lo and hi
    if (works(mid)) hi = mid;              // mid works: the first T is mid or to its left
    else lo = mid;                         // mid fails: the first T is to its right
}
// answer: hi
```

**Maximise:** the largest k that works. `T T T F F F`: find the last T.

```cpp
long long lo = LOW, hi = HIGH + 1;         // lo: surely works (never tested); hi: surely fails (never tested)
while (lo + 1 < hi) {
    long long mid = lo + (hi - lo) / 2;
    if (works(mid)) lo = mid;
    else hi = mid;
}
// answer: lo
```

Why it's safe:

- `mid` is always strictly between `lo` and `hi`, so `works` is never called on the starting values, and every step shrinks the gap: it can't loop forever.
- When `lo + 1 == hi`, the boundary sits exactly between them. Minimising, the answer is in `hi`; maximising, it's in `lo`.
- `lo + (hi - lo) / 2` is the middle, written so it can't overflow the way `(lo + hi) / 2` can.
- On an array, start with `lo = -1, hi = n`. With `works(i) = (v[i] >= x)`, the minimise version gives exactly `lower_bound`.

> [!WARNING]
> **The starting values are never tested, so they must be true to their role.** Minimising, `hi = HIGH` assumes `HIGH` works; maximising, `lo = LOW` assumes `LOW` works. If you're not sure, start one step further out (`hi = HIGH + 1`, or `lo = LOW - 1`) and treat an answer that's still on that value as "nothing works", as the array version does with `hi = n`.

The three steps for any problem:

1. Decide the search space, and set `lo` and `hi` on the correct sides of the boundary: on values you **know** fail or work, or just outside the space.
2. Write a monotonic `works(k)`: F…F then T…T (minimise), or T…T then F…F (maximise).
3. Read the answer from `hi` when minimising, from `lo` when maximising.

Example: the smallest k with `k * k >= n` uses `works(k) = (k * k >= n)`. The same template solves problems like "minimum ship capacity to deliver everything in D days", where `works(k)` simulates the days with capacity k.

### Comparators, one more time

- **A range sorted in descending order:** pass the same comparator to the search as to the sort, e.g. `greater<int>()` in both. `lower_bound` then finds the first element ≤ x ([Lesson 3](./Lesson_3_Comparators.md#matching-comparators)).
- **Searching structs by one field:** either the mixed-parameter lambdas from [Lesson 3](./Lesson_3_Comparators.md#searching-structs-by-one-field) (`lower_bound` takes `(element, value)`, `upper_bound` takes `(value, element)`), or `partition_point` as above.

---

## Rules that always apply

1. The range must be **sorted**, by the same comparator you search with.
2. Every function finds **"the first T"** of a monotonic question: `lower_bound(x)` asks "≥ x?", `upper_bound(x)` asks "> x?".
3. Nothing qualifies → you get `end()`. Check before dereferencing.
4. Position = `it - v.begin()`. Found exactly = `it != v.end() && *it == x`.
5. Largest ≤ x is `prev(upper_bound(x))`; largest < x is `prev(lower_bound(x))`. Guard with `!= begin()`.
6. Count in [l, r] = `upper_bound(r) - lower_bound(l)`; count of x = `upper_bound(x) - lower_bound(x)`.
7. `binary_search` gives only true/false; use `lower_bound` when you need the position.
8. `partition_point` wants T…T then F…F and returns the first F: phrase it as "still too small?".
9. On a `set` or `map`, use the member `s.lower_bound(x)`; the free function is O(n) there.
10. Binary search on the answer: `while (lo + 1 < hi) { mid = lo + (hi - lo) / 2; if (works(mid)) hi = mid; else lo = mid; }`.

---

<p align="center">
  <a href="./Lesson_10_Sorting.md">← Previous: Lesson 10, the sorting family</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a> &nbsp;·&nbsp; <a href="./Lesson_12_Numeric_Overflow.md">Next: Lesson 12, numeric tools and overflow →</a>
</p>
