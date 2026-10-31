# Lesson 12: numeric tools and overflow

This lesson has two halves. The first is about **overflow**: numbers that grow too big for their type and silently turn into garbage. It's one of the most common reasons a correct-looking solution fails a hidden test. The second is the **`<numeric>`** header: small tools for sums, sequences and number theory.

```cpp
vector<int> v = {1000000000, 1000000000, 1000000000};
long long bad  = accumulate(v.begin(), v.end(), 0);     // overflows: garbage
long long good = accumulate(v.begin(), v.end(), 0LL);   // 3000000000
```

This lesson answers **four questions**:

1. [**How** big can a number get?](#how-big-can-a-number-get)
2. [**Where** does overflow hide?](#where-does-overflow-hide)
3. [**What's** in `<numeric>`?](#whats-in-numeric)
4. [**How** does modular arithmetic work? (Mostly CP)](#how-does-modular-arithmetic-work-mostly-cp)

It ends with the [rules that always apply](#rules-that-always-apply).

---

## How big can a number get?

### Step 1: a variable has a fixed number of bits

Every integer variable is stored in a **fixed number of bits**, and with n bits there are exactly 2ⁿ different patterns. So every type can hold only a limited range of values, however the program uses it.

### Step 2: the ranges you need to know

| Type | Bits | Range | Roughly |
|---|---|---|---|
| `int` | 32 | −2,147,483,648 to 2,147,483,647 | ± 2.1 × 10⁹ |
| `long long` | 64 | about −9.2 × 10¹⁸ to 9.2 × 10¹⁸ | ± 9 × 10¹⁸ |
| `unsigned int` | 32 | 0 to 4,294,967,295 | 0 to 4.3 × 10⁹ |
| `unsigned long long` | 64 | 0 to about 1.8 × 10¹⁹ | 0 to 1.8 × 10¹⁹ |

> [!TIP]
> **Rule of thumb:** a value up to 10⁹ fits in an `int`. But the **product** of two such values (up to 10¹⁸), or the **sum** of many of them, needs a `long long`. 10¹⁸ fits in a `long long`; 10¹⁹ doesn't.

### Step 3: what happens when a value doesn't fit

When a result is too big for its type, that's called **overflow**. The program doesn't crash and doesn't warn you; you just get a wrong number.

```cpp
int x = INT_MAX;     // 2147483647
x = x + 1;           // overflow: in practice -2147483648
```

- **For signed types** (`int`, `long long`), overflow is formally **undefined behaviour**. In practice, the value usually wraps around to a large negative number, but the compiler is allowed to assume it never happens, which can produce even stranger results.
- **For unsigned types**, wrapping around is well defined: `0u - 1` is the largest unsigned value. That's the `v.size() - 1` trap from [Lesson 5](./Lesson_5_Vector_String.md#rules-that-always-apply).

### Step 4: the limits by name

```cpp
INT_MAX, INT_MIN                        // int limits (from <climits>)
LLONG_MAX, LLONG_MIN                    // long long limits
numeric_limits<int>::max()              // the same, for any type (from <limits>)
numeric_limits<long long>::min()
numeric_limits<double>::infinity()      // a true infinity for doubles
```

`bits/stdc++.h` includes both headers. `numeric_limits<T>` is handy when `T` is a type parameter or an alias; otherwise the short macros are fine.

---

## Where does overflow hide?

The key fact behind almost every case below: **C++ picks the type of a calculation from its operands, not from where you store the result.** Two `int`s produce an `int`, even if you then put the result into a `long long`.

### 1. Multiplying two ints

```cpp
int a = 100000, b = 100000;
long long bad  = a * b;          // a * b is computed as an int: overflows BEFORE the assignment
long long good = 1LL * a * b;    // 1LL makes the calculation long long: 10000000000
long long also = (long long)a * b;
```

**The habit:** whenever you multiply values that could be large, write `1LL *` at the front.

### 2. Summing many values

```cpp
int sum = 0;            // 10⁵ values of 10⁹ each = 10¹⁴: overflows an int
long long sum2 = 0;     // fits
for (int x : v) sum2 += x;
```

### 3. accumulate with 0

`accumulate` keeps its running total in the **type of the starting value** you pass. The literal `0` is an `int`, so the whole sum is done in `int`:

```cpp
accumulate(v.begin(), v.end(), 0);      // int total: can overflow
accumulate(v.begin(), v.end(), 0LL);    // long long total
accumulate(d.begin(), d.end(), 0.0);    // for doubles: with 0, every partial sum would be cut to an int
```

### 4. The midpoint in binary search

```cpp
int mid = (lo + hi) / 2;          // lo + hi can pass 2.1 × 10⁹
int mid2 = lo + (hi - lo) / 2;    // never larger than hi: safe
```

See [Lesson 11](./Lesson_11_Binary_Search.md#binary-search-on-the-answer-an-algorithm-pattern).

### 5. "Infinity" plus something

```cpp
vector<int> dist(n, INT_MAX);
if (dist[u] + w < dist[v]) ...    // INT_MAX + w overflows to a negative number: wrong comparison
```

Either guard the addition (`dist[u] != INT_MAX && ...`), or choose an "infinity" that leaves room: `const long long INF = 1e18;` stays far from `LLONG_MAX`, so `INF + w` still fits.

### 6. pow

`pow` works with **doubles**, which can't store every large integer exactly. `(long long)pow(10, 18)` may be off by one, and some compilers have returned 24 for `(int)pow(5, 2)`. For integer powers, multiply in a loop, or use [fast power](#fast-power).

### 7. Small things that bite

- `1 << 31` doesn't fit in an `int` (and `1 << 32` and up is undefined); for bit positions 31 and up, write `1LL << k`.
- `-x` and `abs(x)` overflow when `x == INT_MIN`, because +2147483648 doesn't fit in an `int`.
- `1e9` is a **double** literal. `int x = 1e9;` is fine, but in expressions prefer integers like `1000000000` or `(int)1e9`.

---

## What's in `<numeric>`?

### accumulate: fold a range into one value

```cpp
long long sum  = accumulate(v.begin(), v.end(), 0LL);                            // sum
long long prod = accumulate(v.begin(), v.end(), 1LL, multiplies<long long>());   // product
int mx = accumulate(v.begin(), v.end(), INT_MIN, [](int a, int b) { return max(a, b); });
```

The optional last argument says how to combine the running total with each element. The starting value sets the type ([accumulate with 0](#3-accumulate-with-0)).

### iota: fill with consecutive values

`iota(idx.begin(), idx.end(), 0);` fills 0, 1, 2, …. You used it for [index sorts in Lesson 3](./Lesson_3_Comparators.md#index-sorts).

### Prefix sums

**Problem:** many queries of the form "what's the sum of `v[l..r]`?". Adding up each range from scratch costs O(n) per query.

**Idea:** compute the running totals once. Let `p[i]` be the sum of the first `i` elements, so `p[0] = 0`. Then the sum of `v[l..r]` is everything up to `r` minus everything before `l`:

```
v:      3   1   4   1   5
p:   0  3   4   8   9  14        p[i] = v[0] + ... + v[i-1]

sum of v[1..3] = p[4] - p[1] = 9 - 3 = 6     (1 + 4 + 1)
```

These running totals are called **prefix sums**. Building them is O(n), and every query is then **O(1)**:

```cpp
int n = v.size();
vector<long long> p(n + 1, 0);
for (int i = 0; i < n; i++) p[i + 1] = p[i] + v[i];

long long rangeSum = p[r + 1] - p[l];     // sum of v[l..r], inclusive
```

`partial_sum` writes running totals for you: `partial_sum(v.begin(), v.end(), out.begin());`.

> [!WARNING]
> **`partial_sum` adds in the input's type.** If `v` holds `int`s, the running total is an `int` and can overflow, even when `out` is a `vector<long long>`. With large values, use the loop above.

### adjacent_difference: the reverse of prefix sums

`adjacent_difference(v.begin(), v.end(), d.begin());` gives `d[0] = v[0]` and `d[i] = v[i] - v[i-1]`. Running a prefix sum over `d` gets `v` back.

This pair is the basis of the **difference array** trick: to add `x` to every element of `v[l..r]` in O(1), do `d[l] += x; d[r + 1] -= x;` (give `d` one extra slot so `r + 1` is valid), and rebuild `v` with one prefix sum at the end.

### gcd and lcm (C++17)

```cpp
gcd(12, 18);     // 6: the largest number dividing both
lcm(4, 6);       // 12: the smallest number both divide
gcd(0, 7);       // 7

int g = 0;
for (int x : v) g = gcd(g, x);   // gcd of a whole array (gcd(0, x) = x starts it off)
```

> [!WARNING]
> **`lcm` grows fast.** The lcm of two numbers near 10⁹ can be near 10¹⁸, so pass `long long` values. If you write it by hand, divide first: `a / gcd(a, b) * b`, never `a * b / gcd(a, b)`.

---

## How does modular arithmetic work? (Mostly CP)

### The problem

Counting problems often have astronomically large answers (the number of paths in a grid can have hundreds of digits). So the problem asks for the answer **modulo** a fixed number, usually 1000000007 (10⁹ + 7, a prime): the remainder after dividing by it.

```cpp
const int MOD = 1e9 + 7;
```

### The idea: reduce after every step

Remainders survive addition and multiplication: you get the same final remainder whether you reduce at the end or after every step. So **reduce after every operation**, and the numbers never grow past `MOD`.

```cpp
ans = (ans + x) % MOD;                 // addition
ans = 1LL * ans * x % MOD;             // multiplication: two values below ~10⁹ multiply to ~10¹⁸, so 1LL
ans = ((ans - x) % MOD + MOD) % MOD;   // subtraction: % of a negative number is negative in C++
```

> [!WARNING]
> **Subtraction can go negative.** In C++, `(-3) % 7` is `-3`, not `4`. Adding `MOD` before the final `%` brings it back into [0, MOD).

**Division doesn't work this way.** When `MOD` is prime, dividing by `b` means multiplying by `power(b, MOD - 2, MOD)` (its **modular inverse**), using the function below.

### Fast power

Computing bᵉ by multiplying e times is too slow when e is 10¹⁸. Instead, use the **binary digits of e**: for example, 13 = 8 + 4 + 1, so b¹³ = b⁸ · b⁴ · b¹. Square `b` at each step (b, b², b⁴, b⁸, …) and multiply it into the result whenever the matching bit of `e` is 1. That's **O(log e)** multiplications.

```cpp
long long power(long long b, long long e, long long m) {
    long long r = 1;
    b %= m;
    while (e > 0) {
        if (e & 1) r = r * b % m;   // this bit of e is 1: include the current power
        b = b * b % m;              // b, b², b⁴, b⁸, ...
        e >>= 1;                    // move to the next bit
    }
    return r;
}
```

Keep `m` below about 3 × 10⁹, so that `b * b` still fits in a `long long` (10⁹ + 7 is fine). For an exact integer power with no remainder, such as 10¹⁸, a plain loop `r *= b` is simplest, and it's safe as long as the result fits.

---

## Rules that always apply

1. `int` holds about ±2.1 × 10⁹; `long long` about ±9.2 × 10¹⁸. Products of two values near 10⁹, and sums of many, need `long long`.
2. A calculation's type comes from its **operands**, not its destination: write `1LL * a * b`.
3. `accumulate`'s total has the type of its starting value: `0LL` for big sums, `0.0` for doubles.
4. Midpoint: `lo + (hi - lo) / 2`.
5. Pick an "infinity" with room to add to it (`1e18` for `long long`), or guard before adding.
6. Don't use `pow` for integers; use a loop or fast power.
7. `1 << k` is an `int`: for k ≥ 31, write `1LL << k`.
8. Prefix sums: `p[i + 1] = p[i] + v[i]` with `long long p`; sum of `v[l..r]` = `p[r + 1] - p[l]`.
9. `gcd` and `lcm` are in `<numeric>`; for lcm by hand, divide first: `a / gcd(a, b) * b`.
10. Modulo: reduce after every step; `1LL *` for products; `(x % MOD + MOD) % MOD` after subtraction.

---

<p align="center">
  <a href="./Lesson_11_Binary_Search.md">← Previous: Lesson 11, the binary search family</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a>
</p>
