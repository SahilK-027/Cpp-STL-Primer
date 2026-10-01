# Lesson 2: Lambdas

A **lambda** is a *small function written inline*, right where you need it. Instead of defining a separate named function at the top of the file, you write the logic inside the call that uses it.

```cpp
// Without a lambda
bool isEven(int x) {
    return x % 2 == 0;
}

int evenCnt = count_if(v.begin(), v.end(), isEven);

// With a lambda
int evenCnt = count_if(v.begin(), v.end(), [](int x) { return x % 2 == 0; });
```

You will use lambdas constantly, in three places:

- **Custom sort orders:** `sort(v.begin(), v.end(), [](auto& a, auto& b) { ... })`. Lesson 3 covers the rules in depth.
- **Algorithm conditions:** `count_if`, `find_if`, `any_of`, `all_of`, `remove_if`.
- **Recursive helpers** inside `main` or a solution function: DFS, memoised DP, backtracking. They can see local variables (the graph, the memo table) without making them global.

This lesson answers **four questions** about lambdas:

1. [**How** do you write a lambda?](#how-do-you-write-a-lambda)
2. [**How** does a lambda see outside variables?](#how-does-a-lambda-see-outside-variables)
3. [**How** does a lambda plug into STL algorithms?](#how-does-a-lambda-plug-into-stl-algorithms)
4. [**How** do you write a recursive lambda?](#how-do-you-write-a-recursive-lambda) *(Optional, but worth knowing the syntax.)*

---

## How do you write a lambda?

Every lambda has the same shape. Only the capture list and the body are always required.

```text
[capture] (parameters) -> return_type { body }
```

| Part | Meaning | Required? |
|---|---|---|
| `[capture]` | Which outside variables the lambda can use (see [Captures](#captures)) | Yes, but it can be empty: `[]` |
| `(parameters)` | Inputs, just like a normal function | Can be omitted if empty, but write `()` for clarity |
| `-> return_type` | The type it returns | Usually not; the compiler deduces it |
| `{ body }` | The code | Yes |

A lambda is an **object**. Store it with `auto` and call it like a normal function:

```cpp
auto add = [](int a, int b) { return a + b; };
cout << add(3, 4);   // 7
```

#### When must you write the return type?

The compiler deduces the return type from the `return` statements. If two `return` statements give different types, deduction fails:

```cpp
auto f = [](int n) {
    if (n < 0) return 0;      // int
    return 1LL * n * n;       // long long -> COMPILE ERROR: inconsistent types
};

auto g = [](int n) -> long long {
    if (n < 0) return 0;      // converted to long long
    return 1LL * n * n;       // OK
};
```

> [!TIP]
> **Rule of thumb:** write `-> type` when the returns mix types, when the lambda is recursive ([see below](#how-do-you-write-a-recursive-lambda)), or when you want `long long` guaranteed.

---

## How does a lambda see outside variables?

### Captures

A lambda cannot see local variables unless it **captures** them. The capture list `[...]` says which ones, and whether the lambda gets a **copy** or a **reference**.

| Capture | Meaning |
|---|---|
| `[]` | Captures nothing from outside |
| `[x]` | A copy of `x`, taken when the lambda is created |
| `[&x]` | A reference to `x`; sees later changes and can modify it |
| `[=]` | Copies of every local variable the body uses |
| `[&]` | References to every local variable the body uses |
| `[&, x]` | Everything by reference, except `x` by copy |
| `[=, &x]` | Everything by copy, except `x` by reference |

Global variables never need capturing; any lambda can use them.

**A copy is a snapshot.** A by-value capture freezes the value at the moment the lambda is **created**, not when it is called:

```cpp
int x = 10;
auto byCopy = [x]()  { return x; };
auto byRef  = [&x]() { return x; };

x = 99;
cout << byCopy();   // 10  (snapshot taken when x was 10)
cout << byRef();    // 99  (reads the current x)
```

> [!IMPORTANT]
> **By-value copies are read-only.** Inside the body, a copied variable cannot be changed: `[x]() { x++; }` is a compile error. [`mutable`](#mutable) is the exception.

> [!WARNING]
> **The cost of `[=]`.** Copying a vector of 10⁶ elements every time the lambda is created is slow. Capture large containers **by reference**. In an interview, `[&]` is fine. If you want to show care, list the captures explicitly, e.g. `[&adj, &seen]`.

### Mutable

`mutable` lets a lambda modify **its own by-value copies**. The outside variable is never affected.

```cpp
int c = 0;
auto counter = [c]() mutable { return ++c; };

cout << counter() << " ";   // 1
cout << counter() << " ";   // 2  (the lambda's own copy keeps its value between calls)
cout << c << "\n";          // 0  (the original is never touched)
```

Think of the copy as a **private member variable** of the lambda object. It is created once, when the lambda is created, and it persists across calls to that same lambda.

One more detail: **copying the lambda also copies its state.**

```cpp
auto a = counter;   // copies the lambda, including its private c (currently 2)
cout << a();        // 3
cout << counter();  // 3  (an independent copy, also at 2 before this call)
```

> [!NOTE]
> **How often will you use this?** Rarely. To change an outside variable, capture it by reference with `[&]` instead. `mutable` is mainly useful for small generators, such as a lambda passed to `generate` that produces 1, 2, 3, and so on. It appears here so it never surprises you in someone else's code.

---

## How does a lambda plug into STL algorithms?

Many algorithms take a lambda that answers a **yes/no question** about one element. Such a lambda is called a **predicate**. The `_if` suffix tells you an algorithm takes one.

| Algorithm | Returns | Complexity |
|---|---|---|
| `count_if(begin, end, pred)` | The number of elements where `pred` is true | O(n) |
| `find_if(begin, end, pred)` | An iterator to the first match, or `end` | O(n) |
| `any_of(begin, end, pred)` | `true` if at least one element matches | O(n) |
| `all_of(begin, end, pred)` | `true` if every element matches | O(n) |
| `none_of(begin, end, pred)` | `true` if no element matches | O(n) |

```cpp
vector<int> v = {4, 7, 10, 3, 8};

int evens = count_if(v.begin(), v.end(), [](int x) { return x % 2 == 0; });  // 3

auto it = find_if(v.begin(), v.end(), [](int x) { return x > 5; });
if (it != v.end()) cout << *it;   // 7, the first element greater than 5

bool allPos = all_of(v.begin(), v.end(), [](int x) { return x > 0; });     // true

int k = 6;
int bigger = count_if(v.begin(), v.end(), [&](int x) { return x > k; });   // 3, uses the captured k
```

#### Sorting preview

`sort` takes a lambda with **two** parameters that returns `true` when the first should come **before** the second. The full rules (and the traps) are in Lesson 3; for now, just recognise the shape:

```cpp
vector<pair<int, int>> p = {{1, 5}, {2, 3}, {3, 9}};

sort(p.begin(), p.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
    return a.second < b.second;    // sort by second value, ascending
});
```

> [!TIP]
> **Pass big types by `const&`.** Parameters such as `pair`, `string` or `vector` should be `const T&`, so that comparisons do not copy them. For `int`, a plain `int` is fine.

#### Generic lambdas (`auto` parameters)

Since C++14, parameters can be `auto`. The compiler generates the right version for each type, which shortens long type names:

```cpp
sort(p.begin(), p.end(), [](const auto& a, const auto& b) {
    return a.second < b.second;    // same as above, less typing
});
```

---

## How do you write a recursive lambda?

> [!NOTE]
> **This section is optional.**
>
> In my opinion, a normal function is better for recursion than a lambda, mainly because it is less confusing and more readable. It also lets you pass local values or references explicitly as parameters instead of relying on captured or global variables, which is especially useful in DP and graph problems.
>
> I prefer lambdas mostly for comparators and STL algorithms, where they keep the code concise and keep small pieces of logic close to where they are used.
>
> This section is here because recursive lambdas are still worth learning, so you understand the syntax. You don't have to use them in your own code; knowing the technique simply gives you another tool for when you meet it or when a situation calls for it.
>
> **In short:** learn both, but use whichever style makes your code most readable. In my own code, I generally prefer normal functions for recursion and lambdas for comparators and STL algorithms.

A lambda stored with `auto` **cannot call itself** directly. Its type is only known once the whole expression is finished, so inside the body the name cannot be used yet:

```cpp
auto fact = [&](int n) {
    return n <= 1 ? 1 : n * fact(n - 1);   // COMPILE ERROR: fact used before its type is known
};
```

There are two standard fixes.

### Fix 1: `std::function`

Give the variable a concrete type up front with `std::function` (header `<functional>`, included by `bits/stdc++.h`). Capture by reference so the body can see the variable itself.

```cpp
function<int(int)> fact = [&](int n) {
    return n <= 1 ? 1 : n * fact(n - 1);
};

cout << fact(5);   // 120
```

The type reads as `function<return_type(parameter_types)>`. For example, a DFS that returns nothing is `function<void(int, int)>`.

### Fix 2: Pass the lambda to itself (`auto self`)

Add a first parameter that receives the lambda. Inside the body, call `self(...)` instead of the name. When calling from outside, pass the lambda as the first argument.

```cpp
auto fact = [&](auto&& self, int n) -> int {
    return n <= 1 ? 1 : n * self(self, n - 1);
};

cout << fact(fact, 5);   // 120
```

> [!IMPORTANT]
> Write the return type (`-> int`) explicitly. Without it, the compiler may fail to deduce the type of the recursive call.

---

<p align="center">
  <a href="./Lesson_1_Iterators.md">← Previous: Lesson 1, Iterators</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a>
</p>
