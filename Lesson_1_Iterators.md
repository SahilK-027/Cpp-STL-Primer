# Lesson 1: Iterators

An **iterator** is a *position inside a container*.

Every STL algorithm (`sort`, `find`, `lower_bound`, `reverse`) works on a **pair of iterators**, not on the container itself. That is why you write:

```cpp
sort(v.begin(), v.end());   // not sort(v)
```

> [!NOTE]
> Think of an iterator as a **generalised pointer**.
> - For a `vector`, it behaves almost exactly like a pointer into an array.
> - For a `set` or `map`, it points to a node in a balanced tree, so it can only step one node at a time.

This lesson answers **three questions** about iterators:

1. [**Where** do `begin()` and `end()` point?](#where-do-begin-and-end-point) Knowing this prevents off-by-one errors.
2. [**How** do you move an iterator?](#how-do-you-move-an-iterator) With `++`, `--`, `next`, `prev`, `advance` and `distance`.
3. [**What** does each container allow?](#what-does-each-container-allow) For example, `it + k` works on a `vector` but not on a `set`.

Then a [cheat sheet](#cheat-sheet) sums it all up.

---

## Where do `begin()` and `end()` point?

#### Half-open ranges `[begin, end)`

`end()` does **not** point to the last element. It points **one position past** the last element, to a slot that holds nothing.

```cpp
vector<int> v = {10, 20, 30, 40};
```

```text
index:   0     1     2     3     4
v:      [10]  [20]  [30]  [40]  [ ]
         ^                 ^     ^
         |                 |     └── end()        never dereference
         |                 └──────── end() - 1    → 40
         └────────────────────────── begin()      → 10
```

So the last element is `*(v.end() - 1)`, which is `40`.

> [!WARNING]
> Dereferencing `end()` (`*v.end()`) is **undefined behaviour**. It may crash, print garbage, or appear to work.

#### Reverse iterators

Reverse iterators walk the container **backwards**:

- `rbegin()` points to the **last** element, and `++` moves it toward the **front**.
- `rend()` is one position **before** the first element. Never dereference it.

```text
        [ ]   [10]  [20]  [30]  [40]
         ^     ^           ^     ^
         |     |           |     └── rbegin()       → 40
         |     |           └──────── rbegin() + 1   → 30
         |     └──────────────────── rend() - 1     → 10
         └────────────────────────── rend()         never dereference
```

```cpp
vector<int> v = {10, 20, 30, 40};

cout << *v.rbegin();        // 40
cout << *(v.rbegin() + 1);  // 30
cout << *(v.rend() - 1);    // 10  (the last valid reverse position is the first element)
```

---

## How do you move an iterator?

Four functions move and measure iterators: `next`, `prev`, `advance` and `distance`. They work on **every** container, including `set` and `map`.

| Function | What it does | Modifies `it`? |
|---|---|---|
| `next(it, n)` | Returns an iterator `n` steps **forward** (default `n = 1`) | No |
| `prev(it, n)` | Returns an iterator `n` steps **backward** (default `n = 1`) | No |
| `advance(it, n)` | Moves `it` **itself** by `n` steps | **Yes** |
| `distance(a, b)` | Returns the number of steps from `a` to `b` | No |

```cpp
vector<int> v = {10, 20, 30, 40};

cout << *next(v.begin());         // 20
cout << *next(v.begin(), 2);      // 30
cout << *prev(v.end());           // 40 (the last element)
cout << *prev(v.end(), 2);        // 30

auto it = v.begin();
advance(it, 3);                   // it now points to 40
cout << distance(v.begin(), it);  // 3
```

> [!TIP]
> Prefer `next` and `prev` over `advance`. They **return** a new iterator, so the original stays unchanged and the code reads as a single expression.

> [!CAUTION]
> Moving past `end()` or before `begin()` is **undefined behaviour**. `next(v.end())` is never valid.

> [!IMPORTANT]
> **Complexity depends on the container.**
> - On a `vector`, all four functions are **O(1)**.
> - On a `set` or `map`, `next(it, k)`, `prev(it, k)` and `advance(it, k)` take **O(k)**, and `distance(a, b)` takes **O(d)**, where *d* is the number of elements between `a` and `b`.
>
> [What does each container allow?](#what-does-each-container-allow) explains why.

---

## What does each container allow?

Not every iterator supports every operation. What an iterator can do depends on **how its container stores data**. Three categories matter in practice:

| Category | Containers | Allowed operations | Jump `k` steps |
|---|---|---|---|
| **Random access** | `vector`, `deque`, `string`, plain arrays | `++`, `--`, `it + k`, `it - k`, `b - a`, `<` | **O(1)** |
| **Bidirectional** | `set`, `multiset`, `map`, `multimap`, `list` | `++` and `--` only | O(k) via `next` |
| **Forward** | `unordered_set`, `unordered_map`, `unordered_multiset`, `unordered_multimap` | `++` only (no `--`, `prev` or `rbegin`) | O(k) via `next` |

Every iterator supports `++`, `==` and `!=`. That is why loops use `it != s.end()`, **never** `it < s.end()`.

> [!NOTE]
> **Why the difference?** A `vector` is one contiguous block of memory, so jumping `k` positions is simple address arithmetic. A `set` is a balanced binary search tree (a red-black tree). Moving to the next element means following tree links, so there is no shortcut to the k-th element.

> [!NOTE]
> **Why are unordered containers forward-only?** An unordered container is a **hash table**. Elements are scattered into buckets by their hash and linked in one direction only, so there is no backward link and no meaningful "previous" element.
>
> **Rule of thumb:** if a container keeps an order you care about, it can go backward. If it is *unordered*, it can only go forward.

---

## Cheat sheet

#### Vector

For a vector of size `n`, read every iterator as an **index**: `begin()` is `0`, the last element is `n - 1`, and `end()` is `n`, a slot with no element.

| Expression | Index | Notes |
|---|---|---|
| `v.begin() + k` | `k` | Same element as `v[k]` (0-based, not the k-th element counting from 1) |
| `v.end()` | `n` | No element; **never dereference** |
| `prev(v.end())` or `v.end() - 1` | `n - 1` | The last element |
| `v.end() - k` | `n - k` | The k-th element from the back |
| `it - v.begin()` | index of `it` | Converts an iterator to an index |
| `v.end() - it` | `n - index` | Elements remaining, including `*it` |
| Inclusive range `[l, r]` | `v.begin() + l` to `v.begin() + r + 1` | The end iterator is one past `r` |

#### Sets and maps

- A `set` **sorts itself**, so insertion order is lost. `*s.begin()` is the smallest element; `*prev(s.end())` and `*s.rbegin()` are the largest.
- Set iterators support **no `+` or `-`** in either direction. Both `s.begin() + 1` and `s.end() - 1` **fail to compile**.
- Use `next` and `prev` instead.

> [!WARNING]
> `next(it, k)` costs **O(k)**, and `distance` costs **O(n)** in the worst case. Calling them inside a loop can cause **TLE** (Time Limit Exceeded).

#### `find`

| Container | Kind | Usage | Complexity |
|---|---|---|---|
| `vector` | Free function | `find(v.begin(), v.end(), x) != v.end()` | O(n) |
| `set` / `map` | Member function | `s.find(x) != s.end()` | O(log n) |

A `vector` has **no member `find`**, so you pass its iterators and the value to the free function. When the value is not found, **both** return `end()`.

---

<p align="center">
  <a href="./README.md">↑ Contents</a> &nbsp;·&nbsp; <a href="./Lesson_2_Lambdas.md">Next: Lesson 2, Lambdas →</a>
</p>
