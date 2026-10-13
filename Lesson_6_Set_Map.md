# Lesson 6: set, multiset, map, multimap

These four containers keep their elements **sorted at all times**. Each is a balanced binary search tree (a red-black tree) inside, so inserting, erasing and finding all cost **O(log n)**, and walking from `begin()` to `end()` visits elements in sorted order.

```cpp
set<int> s = {5, 1, 3, 1};          // {1, 3, 5}: sorted, duplicates removed
multiset<int> ms = {5, 1, 3, 1};    // {1, 1, 3, 5}: sorted, duplicates kept

map<string, int> age;
age["bob"] = 30;                    // key "bob" -> value 30, kept sorted by key
```

| Container | Stores | Duplicates? | Typical use |
|---|---|---|---|
| `set<T>` | values | no | "have I seen this?", sorted unique values |
| `multiset<T>` | values | yes | a sorted bag: sliding-window min/max, medians |
| `map<K, V>` | key → value | keys unique | frequencies, lookups by name or id |
| `multimap<K, V>` | key → value | keys repeat | rarely needed: `map<K, vector<V>>` is usually clearer |

This lesson answers **five questions**:

1. [**How** do you insert and look up?](#how-do-you-insert-and-look-up)
2. [**How** do you erase?](#how-do-you-erase)
3. [**How** do you find neighbours (the next larger or smaller element)?](#how-do-you-find-neighbours)
4. [**How** do you change the order, and when do iterators break?](#how-do-you-change-the-order-and-when-do-iterators-break)
5. [**When** should you use these instead of `unordered_map` / `unordered_set`?](#when-should-you-use-these-instead-of-unordered_map--unordered_set)

It ends with the [rules that always apply](#rules-that-always-apply).

---

## How do you insert and look up?

### Sets

```cpp
set<int> s;
s.insert(5);                          // O(log n)
auto [it, added] = s.insert(5);       // added = false: 5 was already there

s.count(5);                           // 1 or 0 in a set: "is it present?"
s.find(5) != s.end();                 // same check, and gives you the iterator
```

`insert` returns a `pair<iterator, bool>` ([Lesson 4](./Lesson_4_Pair_Tuple.md#where-do-they-show-up)): where the element is, and whether it was newly added.

### Maps

```cpp
map<string, int> cnt;
cnt["apple"]++;                       // frequency counting: missing keys start at 0
cnt["apple"] = 5;                     // set or overwrite

if (cnt.count("kiwi")) { ... }        // check WITHOUT inserting
auto it = cnt.find("kiwi");
if (it != cnt.end()) cout << it->second;   // read without inserting
```

> [!WARNING]
> **`mp[key]` inserts the key if it's missing.** Reading `cnt["kiwi"]` to check a value silently adds `"kiwi" → 0` to the map. That changes `size()`, adds entries to your loops, and costs memory. To check or read without changing the map, use `count` or `find`.

### Inserting without overwriting

| Call | If the key is missing | If the key exists |
|---|---|---|
| `mp[k] = v` | inserts `k → v` | **overwrites** with `v` |
| `mp.insert({k, v})` | inserts `k → v` | does nothing |
| `mp.emplace(k, v)` | inserts `k → v` | does nothing |
| `mp.try_emplace(k, v)` | inserts `k → v` | does nothing, and doesn't even build `v` |

`try_emplace` (C++17) is the cleanest "insert only if absent": unlike `emplace`, it never constructs the value when the key is already there.

### `multiset` counting

`ms.count(x)` in a multiset returns how many copies exist, but it costs O(log n + number of copies), not O(log n). If you only need "is it present?", use `ms.find(x) != ms.end()`.

---

## How do you erase?

There are two forms, and in a `multiset` they do very different things:

| Call | Removes | Cost |
|---|---|---|
| `s.erase(x)` (a value) | **every** element equal to `x` | O(log n + copies) |
| `s.erase(it)` (an iterator) | exactly **one** element, the one at `it` | O(1) amortised |

In a `set` or `map` there's at most one copy, so both feel the same. In a `multiset` the difference is a classic bug:

```cpp
multiset<int> ms = {2, 5, 5, 5, 8};
ms.erase(5);                 // {2, 8}: ALL three 5s are gone

multiset<int> ms2 = {2, 5, 5, 5, 8};
ms2.erase(ms2.find(5));      // {2, 5, 5, 8}: exactly one 5 removed
```

> [!WARNING]
> **Check `find` before erasing by iterator.** If `x` isn't present, `find` returns `end()`, and `erase(end())` is undefined behaviour.
>
> ```cpp
> auto it = ms.find(x);
> if (it != ms.end()) ms.erase(it);   // remove one copy, safely
> ```

For a `map`, `mp.erase(key)` removes that key and its value.

### Erasing while iterating

`erase(it)` returns an iterator to the next element, so use it instead of `++it`, as with vectors in [Lesson 5](./Lesson_5_Vector_String.md#when-iterators-break-vector-rules):

```cpp
for (auto it = s.begin(); it != s.end(); ) {
    if (*it % 2 == 0) it = s.erase(it);   // removes, and moves to the next element
    else ++it;
}
```

Unlike a vector, each erase here is cheap, so the whole loop is O(n log n) or better. (C++20 adds `erase_if(s, pred)` for this.)

---

## How do you find neighbours?

Because the elements are sorted, these containers can answer **"what's the next larger (or smaller) element than x?"** in O(log n). This is their biggest advantage over `unordered_set`.

```cpp
set<int> s = {10, 20, 30};
*s.lower_bound(20);     // 20: first element >= 20
*s.upper_bound(20);     // 30: first element >  20
*s.lower_bound(25);     // 30
s.lower_bound(35);      // s.end(): nothing >= 35
```

> [!WARNING]
> **Use the member function `s.lower_bound(x)`, never `lower_bound(s.begin(), s.end(), x)`.** Both compile and give the same answer, but the free function is **O(n)** on a set: set iterators are bidirectional ([Lesson 1](./Lesson_1_Iterators.md#what-does-each-container-allow)), so it can't jump to the middle and has to walk. The member function uses the tree directly and is O(log n). In a loop, the wrong one turns O(n log n) into O(n²) and a TLE.

### The four neighbour queries

| You want | Code | Guard |
|---|---|---|
| smallest element ≥ x | `auto it = s.lower_bound(x);` then `*it` | `it != s.end()` |
| smallest element > x | `auto it = s.upper_bound(x);` then `*it` | `it != s.end()` |
| largest element ≤ x | `auto it = s.upper_bound(x);` then `*prev(it)` | `it != s.begin()` |
| largest element < x | `auto it = s.lower_bound(x);` then `*prev(it)` | `it != s.begin()` |

The "largest" rows step back one from the "smallest" rows: the element just before the first one that's too big is the biggest one that fits. That's why they need the guard `it != s.begin()`: if nothing is small enough, there's nothing before `it`.

```cpp
set<int> s = {10, 20, 30};
auto it = s.upper_bound(25);            // 30
if (it != s.begin()) cout << *prev(it); // 20: largest element <= 25
```

### Smallest and largest

```cpp
*s.begin();     // smallest
*s.rbegin();    // largest (or *prev(s.end()))
```

Both require a non-empty container.

### Neighbours in maps

The same functions work on keys and return an iterator to a key–value pair:

```cpp
map<int, string> mp = {{10, "a"}, {20, "b"}};
auto it = mp.lower_bound(15);
cout << it->first << " " << it->second;   // 20 b
```

> [!NOTE]
> **Counting elements in a range is not O(log n).** `distance(s.lower_bound(l), s.upper_bound(r))` walks the elements between, so it's O(log n + count). Fast rank queries need `ordered_set` (Lesson 14).

---

## How do you change the order, and when do iterators break?

### Custom order

Everything from [Lesson 3](./Lesson_3_Comparators.md#where-does-the-comparator-go) applies: the comparator is the type in `< >`.

```cpp
set<int, greater<int>> desc;                 // largest first: *desc.begin() is the max
auto cmp = [](const Job& a, const Job& b) {
    if (a.deadline != b.deadline) return a.deadline < b.deadline;
    return a.profit < b.profit;                 // tie-breaker, or equal jobs get dropped
};
set<Job, decltype(cmp)> jobs(cmp);
```

> [!WARNING]
> **A custom order changes what `lower_bound` means.** In `set<int, greater<int>>`, `lower_bound(x)` returns the first element **≤ x**, because "first" now means first in descending order ([Lesson 3, searching](./Lesson_3_Comparators.md#how-do-you-search-with-a-comparator)).

### Elements are read-only

A set's position depends on its value, so you can't change a value in place: `*it = 7;` doesn't compile. To change one, erase it and insert the new value, both O(log n):

```cpp
s.erase(s.find(5));
s.insert(7);
```

In a `map` the **key** is read-only, but the value can be changed freely: `it->second = 99;` is fine.

### When iterators break

Tree nodes never move in memory, so the rules are much kinder than for vectors:

| Operation | What becomes invalid |
|---|---|
| `insert`, `emplace`, `mp[k]` | nothing |
| `erase(it)` / `erase(x)` | only iterators to the erased elements |
| `clear()` | everything |

So you can keep an iterator to an element while inserting others, which you can't do with a vector.

---

## When should you use these instead of `unordered_map` / `unordered_set`?

| You need | Use | Why |
|---|---|---|
| sorted order, min/max, or neighbours (`lower_bound`) | `set` / `map` | only the tree keeps elements sorted |
| only "is it there?" or counts | `unordered_set` / `unordered_map` | O(1) on average, vs O(log n) |
| `pair` or `vector` as the key | `set` / `map` | they only need `<`; unordered ones need a custom hash (Lesson 7) |
| guaranteed worst-case time | `set` / `map` | unordered containers can degrade to O(n) per operation on adversarial input (Lesson 7) |

In interviews, `unordered_map` is the usual default for plain lookups. In contests, `map` is often the safer default, because of anti-hash tests.

---

## Rules that always apply

1. Insert, erase and find are O(log n); elements are always sorted by the comparator.
2. `mp[key]` inserts a missing key. Use `count` or `find` to check or read without changing the map.
3. `mp[k] = v` overwrites; `insert`, `emplace` and `try_emplace` never overwrite.
4. In a `multiset`, `erase(x)` removes every copy; `erase(ms.find(x))` removes one. Check `find` against `end()` first.
5. Use the member `s.lower_bound(x)`, never `lower_bound(s.begin(), s.end(), x)`: O(log n) vs O(n).
6. Largest ≤ x is `prev(s.upper_bound(x))`; largest < x is `prev(s.lower_bound(x))`. Guard with `!= s.begin()`.
7. Elements of a `set` and keys of a `map` are read-only: erase and re-insert to change them.
8. Inserting never invalidates iterators; erasing invalidates only the erased elements.
9. With a custom comparator, also add tie-breakers so different elements aren't treated as equal ([Lesson 3](./Lesson_3_Comparators.md#why-does-a-set-or-map-drop-elements)).

---

<p align="center">
  <a href="./Lesson_5_Vector_String.md">← Previous: Lesson 5, vector and string</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a>
</p>
