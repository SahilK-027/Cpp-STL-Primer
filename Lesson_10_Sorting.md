# Lesson 10: the sorting family

You already know `sort` and how to write any comparator for it ([Lesson 3](./Lesson_3_Comparators.md)). This lesson covers its **three relatives**, and when each one beats a full sort.

```cpp
vector<int> v = {5, 2, 8, 1, 9, 3};
// each line below starts from this original v

sort(v.begin(), v.end());                              // 1 2 3 5 8 9: everything in order
stable_sort(v.begin(), v.end());                       // same, but equal elements keep their input order
partial_sort(v.begin(), v.begin() + 3, v.end());       // 1 2 3 ? ? ?: only the 3 smallest, in order
nth_element(v.begin(), v.begin() + 2, v.end());        // ? ? 3 ? ? ?: just the element that belongs at index 2
```

All four take the same optional comparator as `sort`, with the same rules.

This lesson answers **four questions**:

1. [**How** does `sort` work, and what does it guarantee?](#how-does-sort-work-and-what-does-it-guarantee)
2. [**When** does stability matter?](#when-does-stability-matter)
3. [**What** if you don't need everything sorted?](#what-if-you-dont-need-everything-sorted)
4. [**Which** one should you use?](#which-one-should-you-use)

It ends with the [rules that always apply](#rules-that-always-apply).

---

## How does `sort` work, and what does it guarantee?

### Step 1: split around one element

Pick one element, called the **pivot**. Move everything smaller than it to its left and everything larger to its right. Now the pivot is exactly where it belongs, and the two sides can be sorted separately. Repeating this on each side is **quicksort**. It's fast in practice, but with unlucky pivots it can take **O(n²)**.

### When does quicksort hit O(n²)?

A **good** pivot splits a piece into two halves of about equal size, so after about log n rounds of halving, every piece has size 1. That's log n levels, with O(n) work per level: **O(n log n)**.

A **terrible** pivot splits off almost nothing. If it's the smallest element, one side gets 0 elements and the other gets n − 1. The next round removes only one more element, and so on:

```
level 1:  split n elements      →  0 | pivot | n-1
level 2:  split n-1 elements    →  0 | pivot | n-2
level 3:  split n-2 elements    →  ...
...                               n levels deep
```

The work adds up to n + (n − 1) + … + 1 ≈ n²/2, which is **O(n²)**.

**The classic trigger: an already-sorted array.** A simple quicksort that always picks the **first** element as the pivot fails exactly on sorted input:

```
[1 2 3 4 5]   pivot 1 → nothing smaller → [] 1 [2 3 4 5]
[2 3 4 5]     pivot 2 → nothing smaller → [] 2 [3 4 5]
...           every split peels off just one element
```

Reverse-sorted input fails the same way, and so does an array of all-equal values with a naive splitting step. Those are exactly the inputs that show up in real data and test cases.

**How it's avoided:** pick the pivot at random, or as the median of the first, middle and last elements, which makes bad splits unlikely. `std::sort` goes further, with the safety net in Step 2, which makes O(n log n) a **guarantee**.

> [!TIP]
> If an interviewer asks you to write quicksort, say "random pivot" and mention the sorted-input worst case.

### Step 2: a safety net

To rule out that O(n²) case, `std::sort` watches how deep the splitting goes. If it gets suspiciously deep, it switches to **heapsort** (built on the heap from [Lesson 8](./Lesson_8_Priority_Queue.md#how-does-a-heap-work)), which is O(n log n) always. And for very small pieces it uses simple **insertion sort**, which is fastest on a handful of elements. This combination is called **introsort**.

The result:

- **O(n log n)** comparisons, even in the worst case.
- **In place:** almost no extra memory.
- **Not stable:** while splitting around a pivot, elements jump past each other, so two equal elements can end up in either order.

```cpp
vector<pair<int,char>> p = {{2,'a'}, {1,'b'}, {2,'c'}};
sort(p.begin(), p.end(), [](const auto& x, const auto& y) { return x.first < y.first; });
// {1,'b'} first; then {2,'a'} and {2,'c'} in EITHER order
```

### What `sort` needs

- **Random-access iterators** ([Lesson 1](./Lesson_1_Iterators.md#what-does-each-container-allow)): `vector`, `deque`, `string` and arrays work. A `list` doesn't (it has its own `lst.sort()`), and a `set` is already sorted.
- **A valid comparator:** strict `<` or `>`, never `<=` ([Lesson 3](./Lesson_3_Comparators.md#what-does-a-comparator-return)).

> [!WARNING]
> **`sort` changes the container.** If you still need the original order (for example, to report original positions), sort a copy, or sort indices instead ([Lesson 3](./Lesson_3_Comparators.md#index-sorts)).

> [!NOTE]
> **Comparisons aren't always O(1).** Sorting n strings of length up to L costs O(n log n · L), because each comparison may read the whole string.

To sort part of a vector, use the half-open range from [Lesson 1](./Lesson_1_Iterators.md#half-open-ranges-begin-end): `sort(v.begin() + l, v.begin() + r + 1);` sorts indices `l` to `r` inclusive.

---

## When does stability matter?

### The problem

Orders arrive in time order, each with a priority. You must sort them by priority, high to low, but orders with the **same** priority must stay in their arrival order. `sort` can't promise that: as shown above, equal elements can come out in either order.

### The tool

A sort that keeps equal elements in their input order is called **stable**. `stable_sort` is exactly that:

```cpp
// {id, priority}, listed in arrival order
vector<pair<int,int>> orders = {{1, 2}, {2, 5}, {3, 2}, {4, 5}};
auto cmp = [](const auto& a, const auto& b) { return a.second > b.second; };   // priority high to low

stable_sort(orders.begin(), orders.end(), cmp);
// ids: 2 4 1 3   (2 before 4, and 1 before 3: arrival order kept within each priority)
```

**Cost:** O(n log n), using O(n) extra memory (it's a **merge sort** inside: split in halves, sort each, merge them). If that memory isn't available it still works, in O(n log² n). It's a bit slower than `sort`, so use it only when the order of equal elements matters.

### Two other ways you'll see

- **Add a tie-breaker and use `sort`.** If the elements carry their original index, break ties on it: `if (a.second != b.second) return a.second > b.second; return a.first < b.first;`. That gives the same result with plain `sort`, which is often simpler.
- **Multi-pass sorting.** Sort by the secondary key first, then `stable_sort` by the primary key; stability keeps the secondary order within equal primaries. This gives the same result as a two-key `cmp`, which is what you'd normally write.

---

## What if you don't need everything sorted?

A full sort does O(n log n) work to put **every** element in place. Often you only need a few of them, and doing less work is faster.

### Only the first m, in order: `partial_sort`

**Problem:** show the top 10 of a million scores, in order. Sorting all million is wasted work.

`partial_sort(first, middle, last)` makes the range `[first, middle)` hold the smallest elements, **in sorted order**. The rest end up in no particular order.

```cpp
vector<int> v = {7, 2, 9, 4, 1, 8};
partial_sort(v.begin(), v.begin() + 3, v.end());
// v = 1 2 4 | 7 8 9 in some order   (first 3 sorted; the rest unordered)

partial_sort(v.begin(), v.begin() + 3, v.end(), greater<int>());
// v = 9 8 7 | ...                   (the 3 largest, largest first)
```

**Cost:** O(n log m) for the m smallest. It uses a heap of size m inside, like the [top-k pattern in Lesson 8](./Lesson_8_Priority_Queue.md#top-k-largest-elements).

### Only one position: `nth_element`

**Problem:** find the median, or the k-th smallest value. You don't care about the order of anything else.

`nth_element(first, nth, last)` rearranges the range so that:

- `*nth` is exactly the element that would be there if the range were sorted;
- everything before it is ≤ it, and everything after it is ≥ it, each side in **no particular order**.

```cpp
vector<int> v = {7, 2, 9, 4, 1, 8};
nth_element(v.begin(), v.begin() + 2, v.end());
// v[2] = 4: the 3rd smallest (index 2, counting from 0)
// v[0..1] = {1, 2} in some order; v[3..5] = {7, 8, 9} in some order
```

**How it does less work:** it splits around a pivot like quicksort ([Step 1](#step-1-split-around-one-element) above), but then continues only into the **one side** that contains position `nth`, ignoring the other side. That makes it **O(n) on average**.

Common uses:

```cpp
int n = v.size();
nth_element(v.begin(), v.begin() + n / 2, v.end());
int median = v[n / 2];                     // the middle element (the upper middle when n is even)

nth_element(v.begin(), v.begin() + k, v.end());
// v[0..k-1] are the k smallest, in no particular order

nth_element(v.begin(), v.begin() + (k - 1), v.end(), greater<int>());
int kthLargest = v[k - 1];                 // k-th largest
```

> [!NOTE]
> **`nth_element` vs a heap ([Lesson 8](./Lesson_8_Priority_Queue.md#top-k-largest-elements)).** Both find the k-th largest. `nth_element` is O(n) on average, but needs all the data at once and reorders the array. A heap is O(n log k), but works on a stream of values arriving one at a time and leaves the original data alone.

---

## Which one should you use?

| You need | Use | Cost |
|---|---|---|
| everything in order | `sort` | O(n log n) |
| everything in order, equal elements in input order | `stable_sort` (or `sort` with an index tie-breaker) | O(n log n) |
| the m smallest (or largest), in order | `partial_sort` | O(n log m) |
| the k-th element, or the median | `nth_element` | O(n) average |
| the k smallest, in any order | `nth_element` | O(n) average |
| the top k from a stream, or without changing the data | a heap ([Lesson 8](./Lesson_8_Priority_Queue.md)) | O(n log k) |
| to check whether it's already sorted | `is_sorted(b, e)` | O(n) |

When in doubt, `sort` is almost always fast enough: 10⁶ elements take well under a second. Reach for the others when the problem specifically asks for a k-th element or a top-k, or when the time limit is tight.

### Common slips

- **The middle argument is an iterator, not a count.** `partial_sort(v.begin(), v.begin() + k, v.end())`, not `partial_sort(v.begin(), k, v.end())`.
- **Positions count from 0.** The 3rd smallest is at `v.begin() + 2`.
- **After `nth_element`, neither side is sorted.** Only `*nth` is in its final place.
- **After `partial_sort`, only the first part is sorted.** The rest is in no particular order.
- **Descending order is the same as always:** pass `greater<int>()` or your `cmp` ([Lesson 3](./Lesson_3_Comparators.md)).

---

## Rules that always apply

1. `sort` is O(n log n) even in the worst case, works in place, and is **not stable**.
2. `sort` needs random-access iterators: `vector`, `deque`, `string`, arrays. A `list` uses `lst.sort()`.
3. `stable_sort` keeps equal elements in input order; adding an index tie-breaker to `sort` does the same.
4. `partial_sort(b, b + m, e)` sorts only the m smallest into the front; the rest is unordered. O(n log m).
5. `nth_element(b, b + k, e)` puts the correct element at index k, smaller-or-equal ones before it and greater-or-equal ones after, each side unordered. O(n) average.
6. Median: `nth_element(v.begin(), v.begin() + n / 2, v.end()); v[n / 2];`
7. All four take the same comparator as `sort`, with the same rules ([Lesson 3](./Lesson_3_Comparators.md)).
8. They all modify the range: sort a copy, or sort indices, if you need the original.

---

<p align="center">
  <a href="./Lesson_9_Deque_Bitset.md">← Previous: Lesson 9, deque and bitset</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a>
</p>
