# Lesson 5: vector and string

A **`vector`** is a dynamic array: one contiguous block of memory that grows as you add elements. A **`string`** is essentially a `vector<char>` with text functions on top. You already know the basics; this lesson covers the details that cause bugs and TLEs.

```cpp
vector<int> v = {3, 1, 4};
v.push_back(1);                  // {3, 1, 4, 1}

string s = "abc";
s += 'd';                        // "abcd"
cout << s.substr(1, 2);          // "bc"
```

This lesson answers **five questions**:

1. [**How** do you create a `vector`?](#how-do-you-create-a-vector)
2. [**What** is the difference between size and capacity?](#what-is-the-difference-between-size-and-capacity)
3. [**How** do you add and remove elements, and when do iterators break?](#how-do-you-add-and-remove-elements-and-when-do-iterators-break)
4. [**How** do you build, edit, search and convert strings?](#how-do-you-build-edit-search-and-convert-strings)
5. [**How** do you parse input?](#how-do-you-parse-input)

It ends with the [rules that always apply](#rules-that-always-apply).

---

## How do you create a `vector`?

```cpp
vector<int> a;                        // empty
vector<int> b(5);                     // {0, 0, 0, 0, 0}: 5 elements, value-initialised
vector<int> c(5, -1);                 // {-1, -1, -1, -1, -1}
vector<int> d = {3, 1, 4};            // exactly these elements
vector<int> e(d.begin(), d.end());    // copy of a range (any container)

vector<vector<int>> grid(n, vector<int>(m, 0));   // n rows, m columns, all 0
```

> [!WARNING]
> **Parentheses vs braces.** `vector<int> x(5, 1)` is five 1s, but `vector<int> y{5, 1}` is two elements, 5 and 1. Braces always mean "these exact elements".

**Reset to a new size and value:** `v.assign(n, 0);` replaces the whole contents with `n` zeros. It is the clean way to reset a vector between test cases.

---

## What is the difference between size and capacity?

- **`size()`**: how many elements the vector holds.
- **`capacity()`**: the maximum number of elements the vector can hold with its currently allocated memory, before it must reallocate.

When `push_back` finds the vector full, it allocates a new block (a multiple of the old size: 2× in GCC and Clang, 1.5× in Microsoft's library; it depends on the compiler's library, not the operating system), moves every element into it, and frees the old one. Because the capacity grows by a multiple each time, this happens rarely, so `push_back` is **O(1) on average** (amortised).

```cpp
vector<int> v;
for (int i = 0; i < 5; i++) {
    v.push_back(i);
    // size: 1 2 3 4 5    capacity (typically): 1 2 4 4 8
}
```

### reserve vs resize

These are easy to mix up, and mixing them up is a classic bug:

| Call | Changes size? | Changes capacity? | Use when |
|---|---|---|---|
| `v.reserve(n)` | no | yes, to at least `n` | you'll `push_back` about `n` elements and want to avoid reallocations |
| `v.resize(n)` | yes, to exactly `n` (new slots are 0) | if needed | you want `n` usable slots to fill with `v[i] = ...` |

```cpp
vector<int> a;
a.reserve(3);
a[0] = 5;          // BUG: size is still 0, so a[0] doesn't exist (undefined behaviour)
a.push_back(5);    // correct after reserve

vector<int> b;
b.resize(3);       // {0, 0, 0}
b[0] = 5;          // correct after resize
```

> [!NOTE]
> **`clear()` keeps the memory.** It sets size to 0 but leaves capacity unchanged. That is usually what you want when reusing a vector across test cases.

### `push_back` vs `emplace_back`

- **`push_back(x)`** takes an element that already exists and copies (or moves) it in.
- **`emplace_back(args...)`** builds the element directly inside the vector from its constructor arguments.

```cpp
vector<pair<int,int>> v;
v.push_back({1, 2});      // builds a pair, then moves it in
v.emplace_back(1, 2);     // builds the pair in place: no braces
```

For `int`, `pair<int,int>` and other small types, the speed difference is negligible. Use whichever reads better; `emplace_back` just saves the braces.

### Adding several elements at once: `insert`

`push_back` and `emplace_back` both add exactly **one** element. (`emplace_back(1, 2)` builds a single pair from the arguments 1 and 2.) To add several at once, use `insert`:

```cpp
v.insert(v.end(), w.begin(), w.end());   // append every element of w
v.insert(v.begin() + i, 3, 0);           // three 0s at index i
v.insert(v.end(), {7, 8, 9});            // a brace list
```

Inserting at the end costs O(number added). Inserting anywhere else also shifts everything after the position, so it costs O(n).

---

## How do you add and remove elements, and when do iterators break?

### Costs

A vector is one contiguous block, so changing the **end** is cheap, but changing the **middle** shifts everything after it.

| Operation | Cost | Why |
|---|---|---|
| `push_back`, `pop_back`, `emplace_back` | O(1) | only the end changes |
| `v[i]`, `v.back()`, `v.front()` | O(1) | direct address |
| `insert(it, x)`, `erase(it)` | O(n) | every element after `it` shifts by one |
| `insert` / `erase` at the front | O(n) | the whole vector shifts |

Precisely, `insert(it, x)` and `erase(it)` cost O(number of elements after `it`), because only those shift. At `end()` nothing shifts, so `v.insert(v.end(), x)` is O(1) amortised, the same as `push_back`, and erasing the last element is O(1), like `pop_back`. The table's O(n) is the middle and front.

Need fast inserts and removals at the front? That is a `deque` (Lesson 9).

### Erasing

```cpp
v.erase(v.begin() + i);                    // remove index i: O(n)
v.erase(v.begin() + l, v.begin() + r);     // remove indices [l, r): half-open, like Lesson 1
```

**Remove every copy of a value: the erase-remove idiom.** `remove` moves the elements to keep to the front and returns where the leftovers start; `erase` then cuts them off. One pass, O(n):

```cpp
v.erase(remove(v.begin(), v.end(), 0), v.end());                      // remove all 0s
v.erase(remove_if(v.begin(), v.end(), [](int x) { return x < 0; }), v.end());   // remove all negatives
```

> [!WARNING]
> **`remove` alone doesn't shrink the vector.** It only rearranges elements; the size stays the same until you call `erase`. (C++20 adds `erase(v, 0)` and `erase_if(v, pred)` as one-call versions.)

**Order doesn't matter? Remove in O(1):** overwrite the element with the last one, then pop the last.

```cpp
v[i] = v.back();
v.pop_back();
```

### When iterators break (vector rules)

An iterator, pointer or reference into a vector points at a memory address. Anything that moves elements makes it **invalid**: using it afterwards is undefined behaviour.

| Operation | What becomes invalid |
|---|---|
| `push_back` / `emplace_back` / `insert` that **exceeds capacity** | everything: the whole vector moved to a new block |
| `push_back` within capacity | only `end()` |
| `insert(it, x)` within capacity | iterators at and after `it` |
| `erase(it)` | iterators at and after `it` |
| `pop_back` | the last element and `end()` |
| `clear`, `assign` | everything |

The bug this causes in practice:

```cpp
int& first = v[0];
v.push_back(42);        // may reallocate
cout << first;          // BUG: first may point at freed memory

for (int x : v) {
    if (x == 1) v.push_back(1);   // BUG: range-for uses iterators, push_back can invalidate them
}
```

> [!TIP]
> **Erasing while looping.** `erase` returns an iterator to the element after the one removed, so use it instead of `++`:
>
> ```cpp
> for (auto it = v.begin(); it != v.end(); ) {
>     if (*it % 2 == 0) it = v.erase(it);   // erase returns the next valid iterator
>     else ++it;
> }
> ```
>
> This works, but each `erase` is O(n), so the loop is O(n²). For vectors, prefer the erase-remove idiom above.

---

## How do you build, edit, search and convert strings?

A `string` is a vector of `char`, so everything above applies: the end is cheap, the middle is O(n), and the same invalidation rules hold.

### Building

```cpp
string s;
s += 'a';            // append a char: O(1) amortised
s.push_back('b');    // same thing
s += "cd";           // append a string: O(length of "cd")
reverse(s.begin(), s.end());   // "dcba": algorithms work on strings
```

> [!WARNING]
> **`s = s + x` in a loop is O(n²).** `s + x` builds a brand-new string (copying all of `s`) every time. Use `s += x`, which appends in place.
>
> ```cpp
> for (char c : t) s = s + c;   // O(n²): copies s on every step
> for (char c : t) s += c;      // O(n)
> ```

### Substrings and searching

```cpp
string s = "hello world";
s.substr(6);            // "world": from index 6 to the end
s.substr(0, 5);         // "hello": start, LENGTH (not end index)

s.find("o");            // 4: first match
s.find("o", 5);         // 7: first match at index 5 or later
s.rfind("o");           // 7: last match
s.find("xyz");          // string::npos: not found

if (s.find("wor") != string::npos) { /* contains "wor" */ }
```

> [!WARNING]
> **`substr(pos, len)` takes a length, not an end index.** And it **copies**, so it costs O(len). Calling it inside a loop can quietly make your solution O(n²).

**In the middle:** `s.insert(pos, "xy")` and `s.erase(pos, len)` are O(n), like vector.

### Characters

Characters are small integers, so arithmetic on them is the standard trick:

```cpp
int d = c - '0';            // '7' -> 7
char ch = '0' + d;          // 7 -> '7'
int idx = c - 'a';          // 'a'..'z' -> 0..25
char letter = 'a' + i;      // 0..25 -> 'a'..'z'

int cnt[26] = {};
for (char c : s) cnt[c - 'a']++;     // letter frequencies
```

Helpers from `<cctype>`: `isdigit(c)`, `isalpha(c)`, `islower(c)`, `isupper(c)`, `tolower(c)`, `toupper(c)`. `tolower` and `toupper` return an `int`, so write `char(tolower(c))` or assign the result to a `char`.

### Converting

| From | To | Code |
|---|---|---|
| number | string | `to_string(42)` |
| string | int | `stoi("123")` |
| string | long long | `stoll("12345678901")` |
| char | string | `string(1, c)` |

`stoi` throws an exception if the text isn't a number, or if the value doesn't fit in an `int`. Use `stoll` for large values.

### Comparing

`==`, `<` and `sort` compare strings **lexicographically**, character by character ([Lesson 3](./Lesson_3_Comparators.md)). Uppercase letters come before lowercase: `"Z" < "a"` is true.

---

## How do you parse input?

Most contest input is read with `cin >>`, which skips spaces and newlines automatically. Interview questions, though, often hand you **one string** to take apart: `"3,1,4"`, `"/a/b/../c"`, a sentence. These are the tools.

### Reading a whole line

```cpp
string line;
getline(cin, line);      // the whole line, spaces included, without the newline
```

> [!WARNING]
> **Mixing `cin >>` and `getline`.** After `cin >> n;` the newline is still waiting in the input, so the next `getline` returns an empty line. Skip it first:
>
> ```cpp
> cin >> n;
> getline(cin >> ws, line);   // ws skips the leftover newline and spaces
> ```

### Splitting with `stringstream`

A `stringstream` lets you read from a string exactly as if it were `cin`.

```cpp
// split by whitespace
stringstream ss("10 20 30");
vector<int> nums;
int x;
while (ss >> x) nums.push_back(x);          // {10, 20, 30}

// split by any delimiter
stringstream ss2("a,b,,c");
vector<string> parts;
string token;
while (getline(ss2, token, ',')) parts.push_back(token);   // {"a", "b", "", "c"}
```

The delimiter version keeps **empty pieces** (`""` between the two commas). For a path like `"/a/b/../c"`, split on `'/'` and skip the empty pieces and `"."`.

### `string_view` (C++17)

A `string_view` is a read-only **window** into an existing string: a pointer plus a length, with no copy. Its `substr` is O(1), because it only moves the window.

```cpp
string s = "hello world";
string_view sv = s;
string_view w = sv.substr(6);    // "world": no copy
```

Use it to pass parts of a string to a function without copying them.

> [!WARNING]
> **A `string_view` must not outlive its string.** It doesn't own the text. `string_view sv = s + "!";` views a temporary string that is destroyed at the end of the line, leaving `sv` pointing at freed memory. In contests you rarely need `string_view`; in interviews, know what it is and this one danger.

---

## Rules that always apply

1. The end of a vector or string is O(1); the middle (`insert`, `erase`) is O(n).
2. `reserve(n)` changes capacity only; `resize(n)` changes size. After `reserve`, use `push_back`, not `v[i]`.
3. `v(5, 1)` is five 1s; `v{5, 1}` is the two elements 5 and 1.
4. Remove values with erase-remove: `v.erase(remove(...), v.end())`. `remove` alone doesn't shrink anything.
5. Any insert that exceeds capacity invalidates **every** iterator, pointer and reference. Don't hold `&v[i]` across a `push_back`, and don't `push_back` inside a range-for over the same vector.
6. Use `s += x`, never `s = s + x`, in loops.
7. `substr(pos, len)` takes a length and copies: O(len).
8. `find` returns `string::npos` when not found; compare against it, not `-1`.
9. After `cin >> n`, use `getline(cin >> ws, line)`.
10. `v.size()` is unsigned: on an empty vector, `v.size() - 1` is a huge number, not `-1`. Write `i + 1 < v.size()` instead of `i < v.size() - 1`, or cast with `(int)v.size()`.

---

<p align="center">
  <a href="./Lesson_4_Pair_Tuple.md">← Previous: Lesson 4, pair and tuple</a> &nbsp;·&nbsp; <a href="./README.md">↑ Contents</a>
</p>
