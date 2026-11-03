# Lesson 14: pitfall review

This final lesson teaches nothing new. It puts the whole course on one page, organised for fast lookup before an interview or contest: the tables you'd otherwise rebuild from five different lessons, plus the traps that cost the most.

This review answers **five questions**:

1. [**When** do iterators break? (One table for every container.)](#when-do-iterators-break)
2. [**What** does everything cost?](#what-does-everything-cost)
3. [**Which** container should you pick?](#which-container-should-you-pick)
4. [**Which** mental models solve most problems?](#which-mental-models-solve-most-problems)
5. [**What** are the pitfalls to check before submitting?](#the-pitfall-checklist)

---

## When do iterators break?

An **iterator** is a position; a **reference or pointer** points at an element itself. Whether each one survives depends on what the container physically does when it changes: copy elements to new memory, shift them, or just relink them.

| Container | What happens inside | Insert / push invalidates | Erase invalidates | References survive an insert? |
|---|---|---|---|---|
| `vector`, `string` | one block; grows by copying to a bigger block ([Lesson 5](./Lesson_5_Vector_String.md#when-iterators-break-vector-rules)) | over capacity: **everything**; within capacity: from the insert point to the end | from the erased point to the end | no, not on reallocation |
| `deque` | fixed-size chunks ([Lesson 9](./Lesson_9_Deque_Bitset.md#when-iterators-break-deque-rules)) | front/back: **all iterators**; middle: **everything** | ends: only the erased element; middle: **everything** | ends: yes; middle: no |
| `set`, `map`, `multiset`, `multimap` | tree nodes that never move ([Lesson 6](./Lesson_6_Set_Map.md#when-iterators-break)) | nothing | only the erased elements | yes |
| `unordered_set`, `unordered_map` | buckets; a rehash re-places every key ([Lesson 7](./Lesson_7_Unordered_Map_Set.md#when-iterators-break)) | on rehash: **all iterators**; otherwise nothing | only the erased elements | yes, even after a rehash |
| `priority_queue`, `stack`, `queue` | adapters with no iterators | — | — | — |

### Safe patterns

```cpp
// Erasing while looping (any container): use what erase returns
for (auto it = c.begin(); it != c.end(); ) {
    if (shouldRemove(*it)) it = c.erase(it);
    else ++it;
}

// Vectors: prefer one pass of erase-remove (O(n)) over erase in a loop (O(n²))
v.erase(remove_if(v.begin(), v.end(), shouldRemove), v.end());

// Growing a vector you hold references into: reserve first, so it never reallocates
v.reserve(expectedSize);
```

> [!WARNING]
> **Never:**
>
> - `push_back` into a vector while range-for looping over it;
> - keep `int& r = v[0];` across a `push_back`;
> - insert into an `unordered_map` while iterating over it (collect the keys first, insert after).

---

## What does everything cost?

### Containers

| Container | Access | Insert / erase | Find | Notes |
|---|---|---|---|---|
| `vector` | `v[i]`: O(1) | end: O(1) amortised; middle: O(n) | O(n); O(log n) if sorted | contiguous, fastest in practice |
| `string` | `s[i]`: O(1) | `+= c`: O(1) amortised; `s + t`: O(n) | `find`: O(n·m) worst | `substr(pos, len)` copies: O(len) |
| `deque` | `d[i]`: O(1) | both ends: O(1); middle: O(n) | O(n) | chunks, not contiguous |
| `set` / `map` (+ multi) | min/max: O(1) | O(log n) | O(log n), plus neighbours | sorted; `ms.count(x)`: O(log n + copies) |
| `unordered_set` / `unordered_map` | — | O(1) average, O(n) worst | O(1) average | no order; rehash is O(n), rare |
| `priority_queue` | `top()`: O(1) | `push` / `pop`: O(log n) | — | build from n elements: O(n) |
| `bitset<N>` | `b[i]`: O(1) | — | — | AND, OR, XOR, shifts and `count`: O(N / 64) |

### Algorithms

| Algorithm | Cost |
|---|---|
| `sort` | O(n log n), guaranteed |
| `stable_sort` | O(n log n) with extra memory; O(n log² n) without it |
| `partial_sort(b, b + m, e)` | O(n log m) |
| `nth_element` | O(n) average |
| `lower_bound`, `upper_bound`, `equal_range`, `binary_search`, `partition_point` | O(log n) on random access; O(n) as free functions on a `set` / `map` |
| `s.lower_bound(x)` (member) | O(log n) |
| `reverse`, `rotate`, `unique`, `remove_if`, `fill`, `count`, `find`, `accumulate`, `min_element` | O(n) |
| `next_permutation` | O(n) per step; n! steps in total |
| `distance(a, b)`, `next(it, k)` on a `set` / `map` | O(k), walks step by step |
| binary search on the answer | O(log(range) · cost of one check) |

> [!TIP]
> **A quick budget check.** About 10⁸ simple operations run in roughly a second. So n = 10⁵ allows O(n log n) easily, O(n√n) usually, and O(n²) never.

---

## Which container should you pick?

Start from what the problem needs to do **repeatedly**, then pick the container that makes that operation cheap.

| You repeatedly need to… | Use | Lesson |
|---|---|---|
| read by index, append at the end | `vector` | [5](./Lesson_5_Vector_String.md) |
| add or remove at both ends | `deque` | [9](./Lesson_9_Deque_Bitset.md) |
| last-in-first-out / first-in-first-out | `stack` / `queue` | [9](./Lesson_9_Deque_Bitset.md) |
| get the best element, add, remove the best | `priority_queue` | [8](./Lesson_8_Priority_Queue.md) |
| keep values sorted and unique; find neighbours (`lower_bound`) | `set` | [6](./Lesson_6_Set_Map.md) |
| the same, with duplicates; remove any one copy; min and max | `multiset` | [6](./Lesson_6_Set_Map.md) |
| map keys to values, in key order or with neighbour queries | `map` | [6](./Lesson_6_Set_Map.md) |
| check membership or count, as fast as possible, no order needed | `unordered_set` / `unordered_map` | [7](./Lesson_7_Unordered_Map_Set.md) |
| store true/false flags with fast whole-set operations | `bitset` | [9](./Lesson_9_Deque_Bitset.md) |
| keep a pair or a few values together | `pair` / `tuple` / `struct` | [4](./Lesson_4_Pair_Tuple.md) |

Two common judgement calls:

- **`unordered_map` or `map`?** In interviews, `unordered_map` is the usual default for plain lookups. Choose `map` when you need order or neighbours, when the key is a pair (no hash needed), or in contests where anti-hash tests are a risk.
- **`priority_queue` or `multiset`?** `priority_queue` is faster and simpler when you only ever touch the best element. Choose `multiset` when you must remove arbitrary elements or need both the min and the max.

---

## Which mental models solve most problems?

These are the ideas that made each lesson click. Each one lets you rebuild the rule instead of memorising it.

| Model | What it gives you | Lesson |
|---|---|---|
| Iterators are indices: `begin()` = 0, last = n − 1, `end()` = n | every off-by-one in ranges and `it - v.begin()` | [1](./Lesson_1_Iterators.md) |
| Write the sign between neighbours in the list you want | any comparator: `1 3 5` → `a < b`, `5 3 1` → `a > b`; ties return `false` | [3](./Lesson_3_Comparators.md) |
| `greater(5, 3)` is true, so 5 goes first | why `greater` sorts descending | [3](./Lesson_3_Comparators.md) |
| `priority_queue` = the sorted list, taken from the back: write the pop order, reverse it, read the signs | any heap comparator; why `greater` gives a min-heap | [3](./Lesson_3_Comparators.md), [8](./Lesson_8_Priority_Queue.md) |
| Object in `( )`, type in `< >` | `greater<int>()` vs `greater<int>`; a lambda in a container needs `decltype(cmp)` + `(cmp)` | [3](./Lesson_3_Comparators.md) |
| Pack the keepers, return where the junk starts, then erase | `remove_if` and `unique`, always with `erase(it, v.end())` | [5](./Lesson_5_Vector_String.md), [13](./Lesson_13_Manipulation_Algorithms.md) |
| Positions move, elements may not | which of iterators and references survive reallocation, rehash or chunk growth | [5](./Lesson_5_Vector_String.md), [7](./Lesson_7_Unordered_Map_Set.md), [9](./Lesson_9_Deque_Bitset.md) |
| Binary search finds the first T of a monotonic question | `lower_bound` = first ≥ x, `upper_bound` = first > x, `partition_point`, search on the answer | [11](./Lesson_11_Binary_Search.md) |
| The largest that fits is just before the first that's too big | largest ≤ x = `prev(upper_bound(x))`; largest < x = `prev(lower_bound(x))` | [6](./Lesson_6_Set_Map.md), [11](./Lesson_11_Binary_Search.md) |
| One pointer on F, one on T, shrink until neighbours | the `lo + 1 < hi` binary search template, with no off-by-one decisions | [11](./Lesson_11_Binary_Search.md) |
| A calculation's type comes from its operands | `1LL * a * b`; `accumulate(..., 0LL)` | [12](./Lesson_12_Numeric_Overflow.md) |
| Replace values by their ranks | coordinate compression: `sort`, `unique` + `erase`, `lower_bound` | [13](./Lesson_13_Manipulation_Algorithms.md) |

---

## The pitfall checklist

Run through this before you submit. Each item is a bug from the course that compiles fine and fails a hidden test.

### Comparators (Lesson 3)

- [ ] Strict `<` or `>` only: never `<=` or `>=`, and a full tie returns `false`.
- [ ] Several keys: `if (a.k1 != b.k1) return a.k1 < b.k1; return a.k2 > b.k2;`, one key per line.
- [ ] In a `set` or `map`, the comparator compares every field that matters, or different elements get dropped.
- [ ] `priority_queue`: written from the reversed pop order.
- [ ] `lower_bound` uses the same comparator the range was sorted with.

### Containers (Lessons 5–9)

- [ ] `reserve(n)` doesn't create elements: use `push_back`, or `resize` before `v[i]`.
- [ ] `mp[key]` inserts a missing key: check with `count` or `find`.
- [ ] In a `multiset`, `erase(x)` removes every copy; `erase(ms.find(x))` removes one (check against `end()` first).
- [ ] On a `set` or `map`, use the member `s.lower_bound(x)`, never the free function.
- [ ] `pq.pop()` returns nothing: read `top()` first. No `top()` or `pop()` on an empty queue.
- [ ] No `push_back` during a range-for over the same vector; no inserting while iterating an `unordered_map`.

### Vectors and strings (Lessons 5, 13)

- [ ] `v.size() - 1` on an empty vector is huge: write `i + 1 < v.size()`.
- [ ] Erase-remove and `unique` need both arguments: `v.erase(remove_if(...), v.end())`.
- [ ] Sort before `unique`.
- [ ] `s += x`, never `s = s + x`, in a loop.
- [ ] `substr(pos, len)` takes a length and copies.
- [ ] `getline(cin >> ws, line)` after `cin >> n`.
- [ ] `memset` only with `0`, `-1` or `0x3f`, and never on a vector.

### Searching (Lessons 6, 11)

- [ ] Check `it != end()` before `*it`, and `it != begin()` before `prev(it)`.
- [ ] The range is sorted first.
- [ ] `binary_search` gives only true or false; use `lower_bound` for the position.

### Numbers (Lesson 12)

- [ ] `1LL * a * b` whenever the product can pass 2 × 10⁹.
- [ ] `accumulate(..., 0LL)` for big sums, `0.0` for doubles.
- [ ] `lo + (hi - lo) / 2` for midpoints.
- [ ] An "infinity" with room to add to it, such as `1e18` in `long long`.
- [ ] `1LL << k` for k ≥ 31; no `pow` for integers.
- [ ] Modulo: reduce after every step, and `(x % MOD + MOD) % MOD` after subtraction.

### Style that prevents bugs

- [ ] `const T&` in comparators and in range-for over strings, vectors or structs.
- [ ] Pass containers to recursive functions by `&` or `const&`, never by value.
- [ ] Reset global containers between test cases.

---

<p align="center">
  <a href="./Lesson_13_Manipulation_Algorithms.md">← Previous: Lesson 13, manipulation algorithms</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a>
</p>
