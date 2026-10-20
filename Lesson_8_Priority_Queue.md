# Lesson 8: priority_queue and heaps

A `priority_queue` always hands you its **"best" element**: the largest by default, or whatever your comparator says is best. You can add elements in any order, and `top()` is always the best one left.

```cpp
priority_queue<int> pq;
for (int x : {3, 9, 1, 7}) pq.push(x);

while (!pq.empty()) {
    cout << pq.top() << " ";    // 9 7 3 1: largest first
    pq.pop();
}
```

You already know the hard part from [Lesson 3](./Lesson_3_Comparators.md#how-do-you-write-a-comparator-for-priority_queue): how to choose the order (`greater` for a min-heap, `cmp` with the three steps for anything custom). This lesson is about what it does inside, what it can't do, and the patterns that use it.

This lesson answers **three questions**:

1. [**How** does a heap work?](#how-does-a-heap-work)
2. [**What** can and can't you do with a `priority_queue`?](#what-can-and-cant-you-do-with-a-priority_queue)
3. [**Which** problem patterns use it?](#which-problem-patterns-use-it)

It ends with the [rules that always apply](#rules-that-always-apply).

---

## How does a heap work?

We'll build it up one idea at a time, naming each idea after it's shown.

### Step 1: you only need the best element

Keeping everything fully sorted is more work than needed: a `priority_queue` only ever shows you **one** element, the best. So it's enough to keep the elements loosely organised, as long as the best one is always easy to find.

### Step 2: parents beat their children

Arrange the elements as a tree in which **every parent is at least as big as each of its children**. Then the biggest element can only be at the very top. This rule is the **heap property**, and a tree that follows it is a **heap** (a max-heap; with "at most as big", a min-heap).

```
            9
          /   \
         7     8
        / \   /
       3   5 6
```

Nothing else is ordered: 7 and 8 are siblings in no particular order, and 6 (under 8) is bigger than 3 and 5 on the other side. Only parent-above-child matters.

### Step 3: store the tree in a plain array

Write the tree level by level, left to right, into an array. Then no pointers are needed: the children of index `i` are always at `2i + 1` and `2i + 2`, and its parent at `(i - 1) / 2`.

```
index:   0  1  2  3  4  5
value:  [9, 7, 8, 3, 5, 6]

children of 1 (the 7):  indices 3, 4  →  3, 5
parent of 5 (the 6):    index 2       →  8
```

That's why a `priority_queue` is declared on top of a `vector<T>` ([Lesson 3](./Lesson_3_Comparators.md#where-does-the-comparator-go)).

### Step 4: push = add at the bottom, then bubble up

Put the new element at the end of the array (the bottom of the tree). If it's bigger than its parent, swap them, and keep swapping upward until its parent is bigger or it reaches the top. This is called **sifting up**.

```
push 10 into [9, 7, 8, 3, 5, 6]:

add at index 6 (child of 8)  →  10 > 8, swap  →  10 > 9, swap  →  10 is the new root

result: [10, 7, 9, 3, 5, 6, 8]
```

The tree has about log₂ n levels, so a push makes at most log n swaps: **O(log n)**.

### Step 5: pop = move the last element to the top, then sink it

Remove the root by overwriting it with the last element of the array. That element is probably too small for the top, so swap it with its **larger** child, and keep going down until both children are smaller. This is **sifting down**, also **O(log n)**.

### Step 6: what each operation costs

| Operation | Cost | Why |
|---|---|---|
| `top()` | O(1) | the best is always at index 0 |
| `push(x)` | O(log n) | sift up through at most log n levels |
| `pop()` | O(log n) | sift down through at most log n levels |
| build from n elements: `priority_queue<int> pq(v.begin(), v.end());` | O(n) | a bottom-up build, faster than n pushes (O(n log n)) |

A min-heap is exactly the same with the comparison flipped, which is why the comparator decides what "best" means (the plate-stack rule from [Lesson 3](./Lesson_3_Comparators.md#how-do-you-write-a-comparator-for-priority_queue)).

---

## What can and can't you do with a `priority_queue`?

### What you can do

```cpp
pq.push(x);          // O(log n)
pq.emplace(a, b);    // build the element in place, e.g. a pair: O(log n)
pq.top();            // the best element: O(1)
pq.pop();            // remove the best: O(log n)
pq.size();  pq.empty();
```

> [!WARNING]
> **`pop()` returns nothing.** It removes the top element but doesn't give it to you. Read it first, then pop:
>
> ```cpp
> int best = pq.top();
> pq.pop();
> ```
>
> Calling `top()` or `pop()` on an empty queue is undefined behaviour, so check `!pq.empty()` first.

### What you can't do

A heap only knows where the best element is; everything else is loosely arranged. So a `priority_queue` has:

- **no iteration**: there's no `begin()` or range-for;
- **no `find`** and no erasing an arbitrary element: only the top can be removed;
- **no changing an element's priority** once it's inside;
- **no `clear()`**: reset it with `pq = {};`.

When you need any of these, the usual answers are [lazy deletion](#dijkstra-and-lazy-deletion) or a `multiset` ([Lesson 6](./Lesson_6_Set_Map.md)):

| You need | `priority_queue` | `multiset` |
|---|---|---|
| best element, add, remove the best | yes, O(log n), and faster in practice | yes, O(log n) |
| remove any element | no (only lazily) | yes: `ms.erase(ms.find(x))` |
| both the min and the max | no (one end only) | yes: `*ms.begin()`, `*ms.rbegin()` |
| iterate in order | no | yes |

### Choosing the order (recap of Lesson 3)

| You want `top()` to be | Declaration |
|---|---|
| the largest | `priority_queue<int> pq;` |
| the smallest | `priority_queue<int, vector<int>, greater<int>> pq;` |
| the smallest pair (Dijkstra's `{dist, node}`) | `priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;` |
| anything custom | `priority_queue<T, vector<T>, decltype(cmp)> pq(cmp);`, with `cmp` from [the three steps](./Lesson_3_Comparators.md#how-do-you-write-a-comparator-for-priority_queue): write the pop order, reverse it, read the neighbours' signs |

---

## Which problem patterns use it?

### Top-k largest elements

**Problem:** find the k largest values among n, where n is huge and k is small.

**Idea:** keep only k candidates at a time. When a (k + 1)-th arrives, throw out the **smallest** candidate, since it can't be among the k largest anymore. "Quickly find the smallest" is a **min-heap**.

```cpp
priority_queue<int, vector<int>, greater<int>> pq;   // min-heap of the k best so far
for (int x : v) {
    pq.push(x);
    if (pq.size() > k) pq.pop();                       // drop the smallest candidate
}
// pq now holds the k largest; pq.top() is the k-th largest
```

Cost: **O(n log k)**, better than sorting everything (O(n log n)) when k is small. The opposite (k smallest) uses a max-heap.

A variant that skips needless work: compare with the smallest candidate first, and only touch the heap when the new value beats it.

```cpp
for (int x : v) {
    if (pq.size() < k) pq.push(x);           // still filling the k slots
    else if (x > pq.top()) {                 // beats the smallest candidate?
        pq.pop();
        pq.push(x);
    }                                        // otherwise ignore x: O(1)
}
```

Same result and the same O(n log k) worst case, but values too small to matter cost only one comparison.

### Merging k sorted lists

**Problem:** merge k sorted lists into one sorted list.

**Idea:** the next output is always the smallest among the k list fronts. Keep the fronts in a min-heap; after taking one, push the next element of the same list.

```cpp
// each entry: {value, which list, position in that list}
priority_queue<tuple<int,int,int>, vector<tuple<int,int,int>>, greater<tuple<int,int,int>>> pq;
for (int i = 0; i < k; i++) if (!lists[i].empty()) pq.push({lists[i][0], i, 0});

vector<int> out;
while (!pq.empty()) {
    auto [val, i, j] = pq.top(); pq.pop();
    out.push_back(val);
    if (j + 1 < lists[i].size()) pq.push({lists[i][j + 1], i, j + 1});
}
```

Cost: **O(N log k)** for N total elements. The tuple puts the value first, so the default tuple order does the work ([Lesson 4](./Lesson_4_Pair_Tuple.md#how-do-they-compare)).

### Dijkstra, and lazy deletion

**Problem:** shortest distances from a source in a graph with non-negative edge weights.

**Idea:** always expand the closest unfinished node next, so keep `{distance, node}` in a min-heap. When a shorter path to a node is found, its entry inside the heap should get a smaller priority, but a `priority_queue` can't change an element. So instead, push a **new** entry with the better distance, and leave the old one inside. When an old, outdated entry later reaches the top, recognise it and skip it.

Leaving outdated entries in the heap and ignoring them when they come out is called **lazy deletion**.

```cpp
vector<long long> dist(n, LLONG_MAX);
priority_queue<pair<long long,int>, vector<pair<long long,int>>, greater<pair<long long,int>>> pq;
dist[src] = 0;
pq.push({0, src});

while (!pq.empty()) {
    auto [d, u] = pq.top(); pq.pop();
    if (d > dist[u]) continue;                 // outdated entry: skip it (lazy deletion)
    for (auto [v, w] : adj[u]) {
        if (dist[u] + w < dist[v]) {
            dist[v] = dist[u] + w;
            pq.push({dist[v], v});             // push the better distance; the old entry stays
        }
    }
}
```

Cost: **O(E log E)**. Distances are `long long`, because sums of many weights can pass the `int` limit ([Lesson 4](./Lesson_4_Pair_Tuple.md#rules-that-always-apply)).

### Running median

**Problem:** numbers arrive one at a time; after each, report the median.

**Idea:** split the numbers into a smaller half and a larger half. The median sits at the boundary: the largest of the smaller half, or the smallest of the larger half. So keep the smaller half in a **max-heap** and the larger half in a **min-heap**, with the smaller half holding the same count or one more.

```cpp
priority_queue<int> lo;                               // smaller half: top = its largest
priority_queue<int, vector<int>, greater<int>> hi;    // larger half: top = its smallest

void add(int x) {
    lo.push(x);
    hi.push(lo.top()); lo.pop();                      // move lo's largest across, so every lo <= every hi
    if (hi.size() > lo.size()) { lo.push(hi.top()); hi.pop(); }   // rebalance sizes
}
double median() {
    return lo.size() > hi.size() ? lo.top() : (lo.top() + (double)hi.top()) / 2;
}
```

Each `add` is O(log n), and `median` is O(1). This is a classic interview question.

---

## Rules that always apply

1. `top()` is O(1); `push` and `pop` are O(log n); building from a range of n elements is O(n).
2. Default is a max-heap; `greater<T>` makes a min-heap; for a custom order, write the pop order, reverse it, read the neighbours' signs.
3. `pop()` returns nothing: read `top()` first. Never call `top()` or `pop()` on an empty queue.
4. No iteration, no `find`, no erasing arbitrary elements, no changing priorities, no `clear()` (use `pq = {};`).
5. To "update" a priority, push a new entry and skip outdated ones when they reach the top (lazy deletion).
6. Need to remove any element, or both min and max? Use a `multiset` instead.
7. Top-k largest: a min-heap capped at size k. Running median: a max-heap for the lower half plus a min-heap for the upper half.
8. Dijkstra: `{dist, node}` with `long long` distances in a min-heap, skipping entries where `d > dist[u]`.

---

<p align="center">
  <a href="./Lesson_7_Unordered_Map_Set.md">← Previous: Lesson 7, unordered_map and unordered_set</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a> &nbsp;·&nbsp; <a href="./Lesson_9_Deque_Bitset.md">Next: Lesson 9, deque and bitset →</a>
</p>
