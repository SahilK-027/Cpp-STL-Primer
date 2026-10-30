# Lesson 9: deque and bitset

Two specialised containers:

- A **`deque`** (pronounced "deck", short for **d**ouble-**e**nded **que**ue) is like a vector that is also fast at the front: adding or removing at **either end** is O(1).
- A **`bitset`** stores a fixed number of true/false values, one bit each, and can run `&`, `|`, `^` and shifts on all of them **about 64 at a time**. (Mostly CP.)

```cpp
deque<int> dq = {2, 3};
dq.push_front(1);         // {1, 2, 3}: O(1); a vector would shift everything
dq.push_back(4);          // {1, 2, 3, 4}

bitset<8> b("10110");     // 00010110
cout << b.count();        // 3: how many bits are 1
```

This lesson answers **three questions**:

1. [**How** does a `deque` work, and what does it cost?](#how-does-a-deque-work-and-what-does-it-cost)
2. [**Which** pattern uses a `deque`?](#which-pattern-uses-a-deque) (Sliding window maximum, a classic interview question.)
3. [**How** does a `bitset` work, and what is it for?](#how-does-a-bitset-work-and-what-is-it-for)

It ends with the [rules that always apply](#rules-that-always-apply).

---

## How does a `deque` work, and what does it cost?

### Step 1: the problem with a vector's front

A vector is one block of memory with its elements packed from the start ([Lesson 5](./Lesson_5_Vector_String.md#what-is-the-difference-between-size-and-capacity)). Adding at the back is cheap, but adding at the front means shifting every element one place right: **O(n)**. Many algorithms (BFS, sliding windows) need to add or remove at both ends.

### Step 2: split the storage into fixed-size chunks

Instead of one big block, keep the elements in many small **fixed-size chunks**, plus a small table that lists the chunks in order:

```
chunk table:   [ c0 ][ c1 ][ c2 ]
                 |     |     |
                 v     v     v
chunks:       [ _ _ 1 2 ] [ 3 4 5 6 ] [ 7 8 _ _ ]
                    ^ front                 ^ back
```

- **Adding at the back:** fill the last chunk's free slots; when it's full, add a new chunk at the end of the table.
- **Adding at the front:** fill the first chunk's free slots backwards; when it's full, add a new chunk at the start of the table.

Either way, **no existing element moves**, so both ends are O(1).

### Step 3: indexing still works

To find element `i`, work out which chunk it's in and where inside that chunk: two lookups, so `dq[i]` is **O(1)**. It's slightly slower than a vector's `v[i]`, because of the extra lookup, and the elements are **not** in one contiguous block.

### The `deque` API

Everything a vector has, plus the front operations:

| Operation | Cost |
|---|---|
| `push_back`, `pop_back`, `push_front`, `pop_front` (and `emplace_` versions) | O(1) |
| `front()`, `back()`, `dq[i]` | O(1) |
| `insert` / `erase` in the middle | O(n) |
| `size()`, `empty()`, `clear()` | O(1) / O(1) / O(n) |

```cpp
deque<int> dq;
dq.push_back(2);          // {2}
dq.push_front(1);         // {1, 2}
dq.push_back(3);          // {1, 2, 3}
dq.pop_front();           // {2, 3}
cout << dq.front() << " " << dq.back() << " " << dq[1];   // 2 3 3
```

> [!NOTE]
> **`queue` and `stack` use a `deque` inside.** `queue<int>` is a deque that only allows push at the back and pop at the front; `stack<int>` only allows both at the back. So every BFS with a `queue` already uses a deque. Use `deque` directly when you need both ends, or indexing.

### When iterators break (deque rules)

Because elements never move when an end grows, **references survive**. Iterators are a different story, since they also depend on the chunk table:

| Operation | Iterators | References and pointers |
|---|---|---|
| `push_front` / `push_back` | **all invalid** | still valid |
| `pop_front` / `pop_back` | only the removed element | only the removed element |
| `insert` / `erase` in the middle | **all invalid** | **all invalid** |

Compare with the vector rules in [Lesson 5](./Lesson_5_Vector_String.md#when-iterators-break-vector-rules) and the hash-table rules in [Lesson 7](./Lesson_7_Unordered_Map_Set.md#when-iterators-break).

---

## Which pattern uses a `deque`?

### Sliding window maximum

**Problem:** given an array `a` and a window size `k`, report the maximum of every window of `k` consecutive elements. Checking each window from scratch is O(n·k); we want **O(n)**.

**Step 1: some elements can never win.** Suppose an older element is ≤ a newer one. The newer one is at least as big, and it stays in the window longer. So the older one can never be the maximum of any future window: throw it away.

**Step 2: keep only the candidates.** After throwing those away, the remaining candidates, in arrival order, have **decreasing** values: each one is larger than every candidate that came after it. So the **front** candidate is the current maximum.

**Step 3: what happens at each new element `a[i]`.**

1. **From the back:** remove candidates that are ≤ `a[i]` (they can never win now), then add `i`.
2. **From the front:** remove the front candidate if it has slid out of the window (its index is ≤ `i - k`).
3. Once the first full window is reached (`i >= k - 1`), the front candidate is the answer.

Removing from both ends is exactly what a deque is for. Because the values inside stay in decreasing (monotonic) order, this is called a **monotonic deque**.

```cpp
vector<int> slidingMax(const vector<int>& a, int k) {
    deque<int> dq;                        // indices of candidates; their values decrease front to back
    vector<int> res;
    for (int i = 0; i < a.size(); i++) {
        while (!dq.empty() && a[dq.back()] <= a[i]) dq.pop_back();   // can never win now
        dq.push_back(i);
        if (dq.front() <= i - k) dq.pop_front();                     // slid out of the window
        if (i >= k - 1) res.push_back(a[dq.front()]);                // front = current max
    }
    return res;
}
```

> [!TIP]
> **Store indices, not values.** You need the index to tell when a candidate has left the window; the value is always `a[index]`.

Trace with `a = {1, 3, -1, -3, 5, 3, 6, 7}` and `k = 3` (the deque shows values, for readability):

| i | a[i] | Removed from the back | Deque after | Window max |
|---|---|---|---|---|
| 0 | 1 | — | 1 | — |
| 1 | 3 | 1 | 3 | — |
| 2 | -1 | — | 3, -1 | 3 |
| 3 | -3 | — | 3, -1, -3 | 3 |
| 4 | 5 | -3, -1, 3 | 5 | 5 |
| 5 | 3 | — | 5, 3 | 5 |
| 6 | 6 | 3, 5 | 6 | 6 |
| 7 | 7 | 6 | 7 | 7 |

**Why O(n):** each index is pushed once and popped at most once, so the total work over the whole array is O(n), even though one step can pop several.

### 0-1 BFS (mostly CP)

In a graph where every edge weighs **0 or 1**, shortest paths don't need Dijkstra ([Lesson 8](./Lesson_8_Priority_Queue.md#dijkstra-and-lazy-deletion)). Use a deque: an edge of weight 0 doesn't increase the distance, so push that neighbour to the **front**; an edge of weight 1 does, so push it to the **back**. The deque then always pops nodes in order of distance, in **O(V + E)** instead of O(E log E).

```cpp
if (dist[u] + w < dist[v]) {
    dist[v] = dist[u] + w;
    if (w == 0) dq.push_front(v);
    else dq.push_back(v);
}
```

---

## How does a `bitset` work, and what is it for?

(Mostly CP. In interviews, bit tricks are written on plain integers: `x >> i & 1`, `x | (1 << i)`.)

### Step 1: a true/false value needs only one bit

A `bool` takes a whole byte (8 bits) to store a single true/false. The computer works on **words** of 64 bits at a time, so one word could hold 64 true/false values, one per bit.

### Step 2: one operation acts on 64 values at once

If two words each hold 64 true/false values, a single machine instruction computes the AND (or OR, XOR) of **all 64 pairs at once**. So set operations on packed bits run about **64× faster** than a loop over `bool`s, and take **8× less memory**.

A container that packs true/false values into words like this, and gives you these whole-word operations, is a **bitset**.

### Declaring one

```cpp
bitset<8> a;                 // 00000000: all bits 0
bitset<8> b(5);              // 00000101: from a number
bitset<8> c("1011");         // 00001011: from a string of 0s and 1s
```

> [!WARNING]
> **The size must be a compile-time constant.** `bitset<n>` with a variable `n` read from input doesn't compile. Use the maximum the problem allows: `const int MAXN = 100001; bitset<MAXN> b;`.

When printed, **bit 0 is the rightmost character**: in `bitset<8> b(5)`, which prints `00000101`, bits 0 and 2 are set.

### The `bitset` API

| Operation | Meaning |
|---|---|
| `b[i]`, `b.test(i)` | read bit `i` |
| `b.set(i)`, `b.reset(i)`, `b.flip(i)` | make bit `i` 1, make it 0, toggle it |
| `b.set()`, `b.reset()`, `b.flip()` | the same, for every bit |
| `b.count()` | how many bits are 1 |
| `b.any()`, `b.none()`, `b.all()` | is at least one / none / every bit 1? |
| `a & b`, `a \| b`, `a ^ b`, `~a` | bitwise AND, OR, XOR, NOT, 64 bits per step |
| `a << k`, `a >> k` | shift every bit `k` places |
| `b.to_string()`, `b.to_ulong()`, `b.to_ullong()` | convert to text or a number |

### What it's used for

**Binary representation, quickly:**

```cpp
cout << bitset<8>(13);              // 00001101
cout << bitset<32>(x).count();      // number of 1 bits in x
```

**Subset sum:** which totals can you make from some of the numbers? Let bit `s` mean "sum `s` is reachable". At the start only sum 0 is reachable. For each number `a`, every reachable sum `s` makes `s + a` reachable too, and shifting the whole bitset left by `a` moves **every** reachable `s` to `s + a` at once:

```cpp
bitset<100001> can;
can[0] = 1;                          // sum 0: take nothing
for (int a : nums) can |= can << a;  // old sums, plus old sums + a
// can[s] == 1  means some subset adds up to s
```

That's **O(n · S / 64)** for n numbers and maximum sum S, about 64× faster than the usual `bool` DP.

**Intersections of large sets:** in a graph with a few thousand nodes, store each node's neighbours as a bitset; then `(nbr[u] & nbr[v]).count()` gives their number of common neighbours, 64 checks per step.

> [!NOTE]
> **`vector<bool>` also packs bits**, and its size can be chosen at runtime, but it lacks the fast whole-set operations (`&`, `|`, shifts, `count`) and has some odd behaviours (`v[i]` isn't a real `bool&`). For speed tricks, use `bitset`.

---

## Rules that always apply

1. A `deque` is O(1) at both ends and O(1) for `dq[i]`; the middle is O(n), as with a vector.
2. Its elements aren't in one contiguous block; it's stored as fixed-size chunks.
3. `push_front` / `push_back` invalidate all deque iterators, but not references.
4. `queue` and `stack` are deques with a restricted API; use `deque` directly when you need both ends or indexing.
5. Sliding window maximum: a deque of **indices** with decreasing values. Pop smaller ones from the back, pop the expired one from the front; the front is the max. O(n).
6. 0-1 BFS: weight-0 neighbours go to the front, weight-1 neighbours to the back.
7. A `bitset`'s size must be a compile-time constant; bit 0 is printed rightmost.
8. Bitset operations (`&`, `|`, `^`, shifts, `count`) process 64 bits per step: that's the speedup.
9. Subset sum: `can[0] = 1;` then `can |= can << a;` for each number.

---

<p align="center">
  <a href="./Lesson_8_Priority_Queue.md">← Previous: Lesson 8, priority_queue and heaps</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a> &nbsp;·&nbsp; <a href="./Lesson_10_Sorting.md">Next: Lesson 10, the sorting family →</a>
</p>
