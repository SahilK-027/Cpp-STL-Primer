# Lesson 3: Comparators

A **comparator** is a function that decides the order of two elements. You pass one to `sort`, give one to a `set` or `map`, or plug one into a `priority_queue`, and it tells the STL which element goes first.

```cpp
vector<int> v = {3, 1, 5};

sort(v.begin(), v.end());                                         // 1 3 5  (default: ascending)
sort(v.begin(), v.end(), greater<int>());                         // 5 3 1  (reversed)
sort(v.begin(), v.end(), [](int a, int b) { return a > b; });     // 5 3 1  (custom lambda, same result)
```

This lesson answers **eight questions** about comparators:

1. [**What** does a comparator return?](#what-does-a-comparator-return)
2. [**How** do you write a comparator?](#how-do-you-write-a-comparator)
3. [**Where** does the comparator go?](#where-does-the-comparator-go)
4. [**How** do you write a comparator for `priority_queue`?](#how-do-you-write-a-comparator-for-priority_queue)
5. [**Why** does a `set` or `map` drop elements?](#why-does-a-set-or-map-drop-elements)
6. [**How** do you sort structs and indices?](#how-do-you-sort-structs-and-indices)
7. [**How** do you search with a comparator?](#how-do-you-search-with-a-comparator)
8. [**What** other forms will you see?](#what-other-forms-will-you-see) *(Optional, but you'll meet them in editorials.)*

It ends with the [rules that always apply](#rules-that-always-apply).

---

## What does a comparator return?

A comparator answers exactly one question: **"does `a` go before `b`?"**

> `cmp(a, b)` returns `true` only if `a` must come **strictly before** `b` in the final list. For a tie, it returns `false`.

"Before" means before **in the list you want**, not "smaller". You choose the order; the comparator just describes it.

**The trick:** write the sign you'd see between two neighbours in your final list.

- Want `1, 3, 5`? Neighbours read `1 < 3`, so write `return a < b;`
- Want `5, 3, 1`? Neighbours read `5 > 3`, so write `return a > b;`
- Two equal values have no left and right, so the answer is `false`. That is why it's `<` or `>`, **never** `<=` or `>=`.

If you give no comparator, the STL asks `a < b`, which is why the default order is ascending.

> [!WARNING]
> **Why `<=` crashes.** With `<=`, two equal elements each claim to go first (`160 <= 160` is true both ways), which is an impossible instruction. It is undefined behaviour, and here is what physically happens. `sort` moves each element toward the front of the array while the answer is "yes", and to save time it never checks whether it has reached the start: it relies on the smallest element at the front always answering "no". With `<=`, an element equal to the front one hears "yes", walks off the front of the array, and the program usually crashes (or quietly gives garbage output). This usually happens only on big inputs with many equal values, so small tests pass and a hidden test fails.

---

## How do you write a comparator?

Every custom order is written the same way: a lambda stored in a variable called `cmp`.

```cpp
auto cmp = [](const T& a, const T& b) {
    if (a.key1 != b.key1) return a.key1 < b.key1;   // key 1: neighbours' sign
    return a.key2 > b.key2;                           // key 2: only reached on a tie
};
```

- **One key per line**, each with its own neighbours' sign, so keys can go in different directions.
- `!=` only chooses **which key** to compare. The value you return is always a strict `<` or `>`.
- A full tie falls through to the last line and returns `false`, exactly as the rule requires.

**Example:** events stored as `{time, count}`, sorted by time ascending, and for equal times by count descending. Neither the default nor `greater` can do this, because each moves both keys in the same direction.

```cpp
vector<pair<int,int>> ev = {{3, 5}, {1, 2}, {3, 9}, {1, 7}};   // {time, count}

auto cmp = [](const pair<int,int>& a, const pair<int,int>& b) {
    if (a.first != b.first) return a.first < b.first;   // time: ascending
    return a.second > b.second;                         // count: descending
};
sort(ev.begin(), ev.end(), cmp);
// {1, 7}, {1, 2}, {3, 9}, {3, 5}
```

#### Parameters

- Use `const T&` for anything bigger than a number. `sort` calls `cmp` about `n log n` times, and taking the arguments by value would copy a string or struct on every call.
- Plain `&` (without `const`) can fail to compile in a `set`, because a `set` only hands out `const` elements. So always `const&`.
- Use `[&]` instead of `[]` only when `cmp` needs outside data ([see index sorts](#index-sorts)).

---

## Where does the comparator go?

| Place | Examples | How to pass `cmp` |
|---|---|---|
| **Algorithms** | `sort`, `stable_sort`, `lower_bound`, `max_element` | `cmp` inside `( )` |
| **Containers** | `set`, `multiset`, `map`, `multimap`, `priority_queue` | `decltype(cmp)` inside `< >`, **and** `(cmp)` in the constructor |

```cpp
// Algorithms
sort(v.begin(), v.end(), cmp);
auto best = max_element(v.begin(), v.end(), cmp);

// Containers
set<T, decltype(cmp)> s(cmp);
multiset<T, decltype(cmp)> ms(cmp);
map<T, int, decltype(cmp)> m(cmp);                     // cmp orders the keys
priority_queue<T, vector<T>, decltype(cmp)> pq(cmp);   // see the next section
```

> [!NOTE]
> **Why containers need `decltype`.** The `< >` slots only take **types**, like `int` in `vector<int>`. A lambda is a **value**, and its type has no name you can type, so `decltype(cmp)` names it for you. The container also needs an actual comparator to call, and in C++17 it can't create a lambda from its type alone, so you hand it one with `(cmp)`. That is also why an inline lambda works only in algorithms.

> [!NOTE]
> **Why `priority_queue` needs `vector<T>`.** Its comparator is the **3rd** template argument, and C++ only lets you leave out defaults from the right. So you must write the 2nd argument, `vector<T>`, just to reach the comparator slot.

#### Which containers count

- `vector`, `deque`, `string` and arrays don't keep themselves sorted, so you order them by calling an **algorithm**.
- `unordered_map` and `unordered_set` use a hash and `==`, not a comparator (Lesson 7).

#### Types C++ already knows how to compare

`int`, `long long`, `char`, `string`, `pair`, `tuple` and `vector` have a built-in `<`. `pair` and `tuple` compare the first element, then the second on a tie, and so on; `string` and `vector` compare left to right, like a dictionary. That is why `sort` and `greater` work on them with no `cmp`.

Your own `struct` has no built-in `<`, so it always needs a `cmp` ([or an `operator<`](#what-other-forms-will-you-see)).

#### Shortcuts: when you don't need `cmp`

| Case | Default order: write nothing | Fully reversed: `greater` |
|---|---|---|
| Algorithm | `sort(b, e)` | `sort(b, e, greater<int>())` |
| `set` / `map` | `set<int>` | `set<int, greater<int>>` |
| `priority_queue` | `priority_queue<int>`: **largest** on top | `priority_queue<int, vector<int>, greater<int>>`: **smallest** on top |

> [!TIP]
> **What `greater` really does.** `greater<int>` simply returns `a > b`. When the STL asks "does 5 go before 3?", it calls `greater(5, 3)`, gets `5 > 3 = true`, and puts 5 first. Every pair is checked that way, so the list ends up `5, 3, 1`. The default works the same way with `a < b`. And `priority_queue` takes from the **back** of that same list, which is why `greater` gives a min-heap.

`greater` can only flip the **whole** order, every key in the same direction. For example, `greater<pair<int,int>>` in a `priority_queue` pops `first` ascending, then `second` ascending. Anything else, such as time ascending with count descending, needs `cmp`.

---

## How do you write a comparator for `priority_queue`?

Think of a `priority_queue` as a sorted list where `top()` is always the **last** element, like taking the top plate off a stack. (Inside, it is a heap, not a sorted list, but it pops in exactly this order.) So:

1. Write the order you want elements to come **out** when popping.
2. Rewrite that order **backwards**.
3. Read the neighbours' signs, and that is your `cmp`.

**Example:** pop time ascending, and for equal times count descending, using the events from above.

```cpp
// 1. pop order:  {1,7} {1,2} {3,9} {3,5}
// 2. backwards:  {3,5} {3,9} {1,2} {1,7}
// 3. signs:      time 3 > 1, so >;  count (equal time) 5 < 9, so <
auto cmp = [](const pair<int,int>& a, const pair<int,int>& b) {
    if (a.first != b.first) return a.first > b.first;
    return a.second < b.second;
};
priority_queue<pair<int,int>, vector<pair<int,int>>, decltype(cmp)> pq(cmp);
```

This gives the same code as writing the `sort` comparator for the pop order and flipping every sign.

---

## Why does a `set` or `map` drop elements?

A `set` or `map` treats two elements as **equal** when `cmp` can't order them, and keeps only one. If `cmp` looks at only some fields, different elements get dropped silently:

```cpp
struct Job { int deadline, profit; };

auto cmp = [](const Job& a, const Job& b) { return a.deadline < b.deadline; };
set<Job, decltype(cmp)> s(cmp);

s.insert({5, 100});
s.insert({5, 200});    // same deadline = equal for this cmp: NOT inserted
```

> [!IMPORTANT]
> **Fix it** with a tie-breaker on the other fields, or use `multiset` / `multimap` if duplicates are allowed:
>
> ```cpp
> auto cmp = [](const Job& a, const Job& b) {
>     if (a.deadline != b.deadline) return a.deadline < b.deadline;
>     return a.profit < b.profit;
> };
> ```

Also note: with a reversed `cmp` or `greater`, `begin()` is the **largest** element.

`sort` has no such problem: it keeps both elements, in no fixed order. `stable_sort` keeps equal elements in their input order.

---

## How do you sort structs and indices?

### Structs

Structs have no default order and no `greater`, so they always need a `cmp`. The pattern is unchanged; only the parameter type differs:

```cpp
// deadline early to late; equal deadlines: profit high to low
auto cmp = [](const Job& a, const Job& b) {
    if (a.deadline != b.deadline) return a.deadline < b.deadline;
    return a.profit > b.profit;
};
sort(jobs.begin(), jobs.end(), cmp);
```

### Index sorts

An index sort orders the **positions** of an array by the values at those positions, without changing the array. Capture the array with `[&]`:

```cpp
vector<int> a = {40, 10, 30, 10};
vector<int> idx(a.size());
iota(idx.begin(), idx.end(), 0);           // fills 0, 1, 2, 3

auto cmp = [&](int i, int j) {             // [&] so cmp can see a
    if (a[i] != a[j]) return a[i] < a[j];  // compare the VALUES
    return i < j;                           // tie: smaller index first
};
sort(idx.begin(), idx.end(), cmp);
// idx = {1, 3, 2, 0}
```

---

## How do you search with a comparator?

### Matching comparators

**Search with the same comparator you sorted with, and sort first.** Whatever you gave `sort`, give `lower_bound`; if you gave nothing, give nothing.

```cpp
vector<int> v = {1, 4, 2, 5, 3};

// Ascending (default): no comparator in either call
sort(v.begin(), v.end());                                      // 1 2 3 4 5
auto it = lower_bound(v.begin(), v.end(), 3);                  // first element >= 3: index 2

// Descending: greater<int>() in both calls
sort(v.begin(), v.end(), greater<int>());                      // 5 4 3 2 1
auto it2 = lower_bound(v.begin(), v.end(), 3, greater<int>()); // first element <= 3: index 2
```

`lower_bound` finds "the first position where `x` could go without breaking the order": the first element ≥ `x` when ascending, the first ≤ `x` when descending.

> [!WARNING]
> **Mismatched comparators fail silently.** The binary search halves in the wrong direction and returns a wrong position, with no error.

### Searching structs by one field

The searched value (an `int`) and the elements (`Job`s) have different types, so this needs its own small lambda with **mixed** parameters. `lower_bound` passes `(element, value)`; `upper_bound` passes `(value, element)`:

```cpp
// jobs sorted by deadline
auto lo = lower_bound(jobs.begin(), jobs.end(), 10,
    [](const Job& j, int x) { return j.deadline < x; });   // first deadline >= 10
auto hi = upper_bound(jobs.begin(), jobs.end(), 10,
    [](int x, const Job& j) { return x < j.deadline; });   // first deadline > 10
```

### `min_element` and `max_element`

Both take the normal "before" comparator, **even for max**:

```cpp
auto best = max_element(jobs.begin(), jobs.end(),
    [](const Job& a, const Job& b) { return a.profit < b.profit; });   // job with the highest profit
```

---

## What other forms will you see?

You don't need to write these, but you'll meet them in editorials and other people's code. They all answer the same question as `cmp`.

- **`greater<T>` and `less<T>`** are ready-made functors from the library: structs whose call operator returns `a > b` or `a < b`. That's why they follow the type/object rule: `greater<int>` in `< >`, `greater<int>()` in `( )`.
- **A functor** is your own named struct with a call operator. Because it has a name, it goes straight into `< >` with no `decltype` and no constructor argument. A lambda is a functor the compiler writes for you, without a name.

  ```cpp
  struct Cmp {
      bool operator()(const Job& a, const Job& b) const { return a.deadline < b.deadline; }
  };
  set<Job, Cmp> s;                          // type in < >
  sort(jobs.begin(), jobs.end(), Cmp());    // object in ( )
  ```

- **`operator<` inside the struct** gives the type a default order. After that, `sort(b, e)`, `set<Job>` and `priority_queue<Job>` need no comparator at all. Worth it when one natural order is used everywhere. It needs the trailing `const`:

  ```cpp
  struct Job {
      int deadline, profit;
      bool operator<(const Job& o) const { return deadline < o.deadline; }
  };
  ```

- **Inline lambdas in algorithms:** `sort(b, e, [](const auto& a, const auto& b) { ... });` is the same as passing `cmp`.
- **The negation trick for heaps:** push `{time, -count}` into a `greater` min-heap to get count descending, and un-negate when reading. Quick in contests; `cmp` is clearer in interviews.
- **C++20** allows `decltype([](...){...})` written directly in `< >`, and drops the `(cmp)` for lambdas with no captures. Most judges and interviews assume C++17, so the two-step form is the safe one.

---

## Rules that always apply

1. `return true` only if `a` goes **strictly** before `b`; ties return `false`. Never `<=` or `>=`.
2. In containers: `decltype(cmp)` in `< >` **and** `(cmp)` in the constructor.
3. `priority_queue`: write the pop order, reverse it, read the neighbours' signs.
4. To give `priority_queue` a comparator, also write `vector<T>` as the 2nd argument.
5. In a `set` or `map`, `cmp` must compare every field that matters, or elements get dropped.
6. `lower_bound` and `upper_bound`: the same comparator the range was sorted with, and sort first.
7. `const T&` parameters for anything bigger than a number; `[&]` only when `cmp` needs outside data.

---

<p align="center">
  <a href="./Lesson_2_Lambdas.md">← Previous: Lesson 2, Lambdas</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a> &nbsp;·&nbsp; <a href="./Lesson_4_Pair_Tuple.md">Next: Lesson 4, pair and tuple →</a>
</p>
