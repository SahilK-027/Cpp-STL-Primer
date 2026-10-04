# Lesson 3: Comparators

A **comparator** is a function that decides the order of two elements. You pass one to `sort`, give one to a `set` or `map`, or plug one into a `priority_queue`, and it tells the STL which element goes first.

```cpp
vector<int> v = {3, 1, 5};

sort(v.begin(), v.end());                                         // 1 3 5  (default: ascending)
sort(v.begin(), v.end(), greater<int>());                         // 5 3 1  (reversed)
sort(v.begin(), v.end(), [](int a, int b) { return a > b; });     // 5 3 1  (custom lambda, same result)
```

This lesson answers **five questions** about comparators:

1. [**What** does a comparator return?](#what-does-a-comparator-return)
2. [**Which** comparator do you need?](#which-comparator-do-you-need)
3. [**Why** write a custom comparator when `greater` exists?](#why-write-a-custom-comparator-when-greater-exists)
4. [**How** do you write a custom comparator?](#how-do-you-write-a-custom-comparator)
5. [**How** do you search with a comparator?](#how-do-you-search-with-a-comparator)

---

## What does a comparator return?

A comparator answers exactly one question:

> `comp(a, b)` returns `true` only if `a` must come **strictly before** `b` in the final ordered output. For a tie, it returns `false`.

**The trick:** write the sign you'd see between two neighbours in your final list.

- Want `1, 3, 5`? Neighbours read `1 < 3`, so write `return a < b;`
- Want `5, 3, 1`? Neighbours read `5 > 3`, so write `return a > b;`
- Two equal values have no left and right, so the answer is `false`. That is why it's `<` or `>`, **never** `<=`.
- Several keys: apply the trick to each key separately.

#### For `priority_queue`

1. Write the order you want elements to come **out** when popping, say `1, 3, 5`.
2. Rewrite that order **backwards**: `5, 3, 1`.
3. Read the neighbours' sign: `5 > 3`, so `return a > b;`, which is `greater`: a **min-heap**.

> [!WARNING]
> **Never use `<=` or `>=`.** A comparator that returns `true` for equal elements breaks the rules `sort` relies on. The result is undefined behaviour: wrong output, or a crash on large inputs.

---

## Which comparator do you need?

Every comparator situation is a combination of **three choices**.

#### Axis 1: What the comparator is for

- **An algorithm:** `sort`, `stable_sort`, `lower_bound`, `max_element`. This includes every `vector`, `deque`, `string` or array: they don't keep themselves sorted, so you order them by calling an algorithm.
- **A self-ordering container:** `set`, `multiset`, `map`, `multimap`. It uses the comparator on every insert, so the comparator is part of its **type**.
- **`priority_queue`:** also self-ordering, but it comes out in the **opposite** order. It gets its own row because of that.

#### Axis 2: What order you want

- **Default:** ascending.
- **Reversed:** descending.
- **Custom:** by one field, by several keys, by a computed value, or by outside data.

Example of a custom order: events stored as `{time, count}`, sorted by time ascending, and for equal times by count descending. Neither the default nor `greater` can do this, because each flips both keys the same way. [Below](#how-do-you-write-a-custom-comparator) shows how to write it.

#### Axis 3: The type of data

- **`int`, `long long`, `string`, `pair`, `tuple`, `vector`:** C++ already knows how to compare them, so all three columns of Axis 2 are open to you. (`pair` and `tuple` compare the first element, then the second, and so on; `string` and `vector` compare left to right, like a dictionary.)
- **Your own `struct`:** C++ doesn't know how to compare it, so there is no default and no `greater`. You are always in the custom column, unless you give the struct an `operator<` ([see below](#unlocking-the-default-column-for-a-struct)).

#### The full table

| Case | Default order | Reversed order | Custom order |
|---|---|---|---|
| Algorithm | `sort(b, e)` | `sort(b, e, greater<int>())` | `sort(b, e, cmp)` |
| `set` / `map` | `set<int>` | `set<int, greater<int>>` | `set<T, decltype(cmp)> s(cmp)` |
| `priority_queue` | `priority_queue<int>`: **largest** on top | `priority_queue<int, vector<int>, greater<int>>`: **smallest** on top | `priority_queue<T, vector<T>, decltype(cmp)> pq(cmp)` |

> [!NOTE]
> **What `greater` really does, and why it gives descending order in `sort` but a min-heap in `priority_queue`.**
>
> In `sort(b, e, greater<int>())`, `greater<int>` simply returns "a greater than b", that is, `a > b`. When the STL asks "does 5 go before 3?", it calls `greater(5, 3)`, gets `5 > 3 = true`, and puts 5 first. Every pair gets checked that way, so the list ends up `5, 3, 1`. The default `less` works the same way with `a < b`, giving `1, 3, 5`.
>
> And `priority_queue` takes from the **back** of that same list, which is why `greater` gives a min-heap.

---

## Why write a custom comparator when `greater` exists?

`greater` can only flip the **whole** default order, every key in the same direction. Anything else needs a custom comparator.

Example: `{time, count}` with time ascending and count descending.

| Comparator | Order produced |
|---|---|
| Default (`less`) | time ↑, count ↑ |
| `greater` | time ↓, count ↓ |
| **Wanted** | time ↑, count ↓ |

Neither built-in gives the wanted order. The same applies to `priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>>`, which pops `first` ascending, then `second` ascending.

---

## How do you write a custom comparator?

### Several keys

Compare the first key; fall through to the next on a tie. This one shape covers almost every custom order.

```cpp
struct Job { int deadline, profit; };

// profit high to low; if equal, deadline early to late
auto cmp = [](const Job& a, const Job& b) {
    if (a.profit != b.profit) return a.profit > b.profit;
    return a.deadline < b.deadline;      // full tie falls through to false: correct
};
sort(jobs.begin(), jobs.end(), cmp);
```

The `{time, count}` example uses the same shape:

```cpp
vector<pair<int,int>> ev = {{3, 5}, {1, 2}, {3, 9}, {1, 7}};   // {time, count}
sort(ev.begin(), ev.end(), [](const auto& a, const auto& b) {
    if (a.first != b.first) return a.first < b.first;   // time: ascending
    return a.second > b.second;                         // count: descending
});
// {1, 7}, {1, 2}, {3, 9}, {3, 5}
```

### Outside data

Capture it with `[&]`. A common case is sorting indices by the values they point to:

```cpp
sort(idx.begin(), idx.end(), [&](int i, int j) { return a[i] < a[j]; });
```

### `priority_queue`

Write the pop order backwards, then use the neighbours' sign. `top()` is the **last** element of the list your comparator describes. So decide the order you want things popped, reverse it, and write the signs you see between neighbours. (This gives the same code as writing the `sort` comparator and flipping every sign.)

```cpp
// pop: earliest deadline first; ties: larger profit first
auto byPop = [](const Job& a, const Job& b) {
    if (a.deadline != b.deadline) return a.deadline > b.deadline;   // sort version: <, flipped
    return a.profit < b.profit;                                    // sort version: >, flipped
};
priority_queue<Job, vector<Job>, decltype(byPop)> pq(byPop);
```

### `set` and `map`

The comparator must tell **every** element apart. Two elements the comparator can't order count as **equal**, and a `set` keeps only one of them:

```cpp
auto byDl = [](const Job& a, const Job& b) { return a.deadline < b.deadline; };
set<Job, decltype(byDl)> s(byDl);

s.insert({5, 100});
s.insert({5, 200});    // same deadline = equal: NOT inserted
```

> [!IMPORTANT]
> **Fix it** with a tie-breaker on the other fields, or use `multiset`. (`sort` keeps both, in no fixed order; `stable_sort` keeps their input order.)

### Unlocking the default column for a struct

If a struct has one natural order you use everywhere, define `operator<` once. Then `sort(b, e)`, `set<Job>` and `priority_queue<Job>` work with no comparator:

```cpp
struct Job {
    int deadline, profit;
    bool operator<(const Job& o) const { return deadline < o.deadline; }   // trailing const required
};
```

---

## How do you search with a comparator?

### Matching comparators

**Search with the same comparator you sorted with.** Whatever comparator you gave `sort`, give the same one to `lower_bound`; if you gave none, give none. The range must be sorted first.

```cpp
vector<int> v = {1, 4, 2, 5, 3};

// Ascending (default): no comparator in either call
sort(v.begin(), v.end());                                      // 1 2 3 4 5
auto it = lower_bound(v.begin(), v.end(), 3);                  // first element >= 3: index 2

// Descending: greater<int>() in both calls
sort(v.begin(), v.end(), greater<int>());                      // 5 4 3 2 1
auto it2 = lower_bound(v.begin(), v.end(), 3, greater<int>()); // first element <= 3: index 2
```

In both cases `lower_bound` means "the first position where `x` could go without breaking the order": the first element ≥ `x` when ascending, the first ≤ `x` when descending.

> [!WARNING]
> **Mismatched comparators fail silently.** If the comparators don't match, the binary search halves in the wrong direction and returns a wrong position, with no error.

### Searching structs by one field

The value (an `int`) and the elements (`Job`s) have different types, so the comparator takes **mixed** arguments. `lower_bound` passes `(element, value)`; `upper_bound` passes `(value, element)`:

```cpp
// jobs sorted by deadline
auto lo = lower_bound(jobs.begin(), jobs.end(), 10,
    [](const Job& j, int x) { return j.deadline < x; });   // first deadline >= 10
auto hi = upper_bound(jobs.begin(), jobs.end(), 10,
    [](int x, const Job& j) { return x < j.deadline; });   // first deadline > 10
```

### `min_element` and `max_element`

Both take the normal "less than", **even for max**:

```cpp
auto best = max_element(jobs.begin(), jobs.end(),
    [](const Job& a, const Job& b) { return a.profit < b.profit; });   // job with the highest profit
```

---

<p align="center">
  <a href="./Lesson_2_Lambdas.md">← Previous: Lesson 2, Lambdas</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a>
</p>
