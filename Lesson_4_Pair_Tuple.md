# Lesson 4: pair and tuple

A **`pair`** holds exactly two values; a **`tuple`** holds any number of values. They keep related values together without defining a `struct`, and they already know how to compare themselves, so `sort`, `set` and `priority_queue` work on them with no `cmp`. You can simply write `greater<Type>` to reverse the order.

```cpp
pair<int, string> p = {25, "Ann"};
auto [age, name] = p;                          // age = 25, name = "Ann"

vector<pair<int,int>> v = {{2, 5}, {1, 9}, {2, 1}};
sort(v.begin(), v.end());                      // {1,9} {2,1} {2,5}: no cmp needed
```

This lesson answers **five questions** about `pair` and `tuple`:

1. [**How** do you create a `pair` or `tuple`?](#how-do-you-create-a-pair-or-tuple)
2. [**How** do you read one?](#how-do-you-read-one)
3. [**How** do they compare?](#how-do-they-compare)
4. [**Where** do they show up?](#where-do-they-show-up)
5. [**When** should you use a `struct` instead?](#when-should-you-use-a-struct-instead)

It ends with the [rules that always apply](#rules-that-always-apply).

---

## How do you create a `pair` or `tuple`?

```cpp
pair<int,int> a = {1, 2};                      // braces: the everyday form
auto b = make_pair(1, 2);                      // types deduced: pair<int,int>
pair c{1, 2};                                  // C++17: types deduced from the braces
tuple<int, string, double> t = {7, "pen", 1.5};

vector<pair<int,int>> v;
v.push_back({3, 4});                           // build the pair, then copy it in
v.emplace_back(3, 4);                          // build it directly inside the vector
```

`emplace_back(3, 4)` passes the arguments straight to the pair's constructor, so no temporary pair is made. For `pair<int,int>` the difference is tiny; both are fine.

---

## How do you read one?

### Structured bindings

This is the form to use. It gives each part a name instead of `.first` / `.second`:

```cpp
auto [age, name] = p;                          // pair
auto [id, label, score] = t;                   // tuple: one name per element

for (const auto& [v, w] : adj[u]) { ... }      // each edge {neighbour, weight}
for (const auto& [key, val] : mp) { ... }      // every map entry
```

The `auto` part decides whether you copy or refer to the original:

| Form | Meaning | Use when |
|---|---|---|
| `auto [a, b] = p;` | copies of the parts | small values you only read |
| `auto& [a, b] = p;` | references: changing `a` changes `p.first` | you want to modify the original |
| `const auto& [a, b] = p;` | read-only references, no copies | loops over containers; strings, vectors, structs |

> [!WARNING]
> **`auto [a, b] = p;` copies.** Changing `a` afterwards does not change `p`. To modify the original, write `auto&`.

### Other forms you'll see

- **`.first` and `.second`**, for pairs only.
- **`get<i>(t)`** reads element number `i`, counting from 0: `get<0>(t)` is the first element, `get<1>(t)` the second. The number must be typed directly in the code, because the compiler needs to know each result's type before the program runs:

  ```cpp
  tuple<int, string, double> t = {7, "pen", 1.5};
  cout << get<1>(t);      // pen

  int i = 1;
  get<i>(t);              // compile error: i is a variable
  ```

- **`tie(a, b) = p;`** copies the parts of a pair into two variables you declared earlier. `ignore` skips a part:

  ```cpp
  int a, b;
  pair<int,int> p = {3, 4};
  tie(a, b) = p;          // a = 3, b = 4
  tie(a, ignore) = p;     // a = 3, the second part is skipped
  ```

  Structured bindings (`auto [a, b] = p;`) do the same while declaring the variables, so you'll mostly meet `tie` in older code.

---

## How do they compare?

**Compare `first`; only if it ties, compare `second`.** A `tuple` continues left to right through every element. This is **lexicographic** order, the same way a dictionary orders words.

```cpp
pair<int,int> a = {1, 5}, b = {1, 3}, c = {2, 0};
a < c;      // true:  1 < 2, second never looked at
b < a;      // true:  first ties, then 3 < 5
a == b;     // false: every part must be equal
```

It is exactly the two-line `cmp` from [Lesson 3](./Lesson_3_Comparators.md#how-do-you-write-a-comparator), already built in:

```cpp
if (a.first != b.first) return a.first < b.first;
return a.second < b.second;
```

So `sort`, `set`, `map` keys and `priority_queue` all work on pairs and tuples with no `cmp`, and `greater<pair<int,int>>` flips both keys.

> [!TIP]
> **Put the sort key first.** When you choose how to store data, put the value you order by in `first`, and the default does the work. That is why Dijkstra stores `{dist, node}`, not `{node, dist}`: the min-heap then pops the smallest distance.
>
> ```cpp
> priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;
> pq.push({dist, node});
> ```

The default only works when every key goes in the **same direction**. For mixed directions (time ascending, count descending), write a `cmp` as in [Lesson 3](./Lesson_3_Comparators.md#how-do-you-write-a-comparator).

---

## Where do they show up?

**Weighted graphs:** each edge is `{neighbour, weight}`.

```cpp
vector<vector<pair<int,int>>> adj(n);
adj[u].push_back({v, w});
for (const auto& [v, w] : adj[u]) { ... }
```

**Grid directions:** the four moves as `{dr, dc}`.

```cpp
vector<pair<int,int>> dirs = {{0,1}, {1,0}, {0,-1}, {-1,0}};
for (auto [dr, dc] : dirs) {
    int nr = r + dr, nc = c + dc;
}
```

**Returning several values from a function:**

```cpp
pair<int,int> minMax(const vector<int>& v) {
    return {*min_element(v.begin(), v.end()), *max_element(v.begin(), v.end())};
}
auto [lo, hi] = minMax(v);
```

**STL functions that return a pair:**

| Call | Returns |
|---|---|
| `minmax_element(b, e)` | `{iterator to min, iterator to max}` |
| `mp.insert({k, v})` | `{iterator to the entry, bool: true if newly inserted}` |
| `equal_range(b, e, x)` | `{lower_bound, upper_bound}` |

```cpp
auto [mnIt, mxIt] = minmax_element(v.begin(), v.end());
cout << *mnIt << " " << *mxIt;

auto [it, inserted] = mp.insert({"apple", 1});
if (!inserted) { /* key already existed; nothing changed */ }
```

> [!NOTE]
> **`map` entries are pairs.** Each element of a `map<K, V>` is a `pair<const K, V>`. That is why `for (auto& [key, val] : mp)` works, and why the key can't be changed through it.

---

## When should you use a `struct` instead?

| Use | When | Why |
|---|---|---|
| `pair` | exactly two values | free comparison, short to write, works with `map` and most STL calls |
| `tuple` | three values, used briefly | free comparison, but `get<2>(t)` reads poorly |
| `struct` | three or more fields, or data that lives long | named fields (`job.deadline`), but needs a `cmp` to be sorted |

In an interview, a `struct` with named fields is usually clearer than a `tuple`. In contests, pairs and tuples save typing and give you sorting for free.

> [!WARNING]
> **Pairs can't be `unordered_map` / `unordered_set` keys out of the box.** They have `==` but no hash, so `unordered_set<pair<int,int>>` fails to compile. `set` and `map` are fine, because they use `<`. Lesson 7 shows the fix.

---

## Rules that always apply

1. Pairs and tuples compare `first`, then `second`, and so on. When every key goes the same direction, `sort`, `set` and `priority_queue` need no `cmp`.
2. Put the value you order by in `first`.
3. `auto [a, b]` copies, `auto& [a, b]` modifies the original, `const auto& [a, b]` reads without copying (use it in loops).
4. Pairs and tuples work as `set` / `map` keys, but not as `unordered_*` keys without a custom hash.
5. Pick types for the range of values: `pair<long long,int>` when sums or distances can pass about 2×10⁹.
6. `get<i>` needs a constant index.
7. Capturing a structured binding in a lambda is only guaranteed from C++20; in C++17, copy into normal variables first if the compiler complains.

---

<p align="center">
  <a href="./Lesson_3_Comparators.md">← Previous: Lesson 3, Comparators</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a>
</p>
