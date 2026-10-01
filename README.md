# C++ STL Primer

A short, practical guide to the C++ Standard Template Library, written for competitive programming and coding interviews. All snippets assume `#include <bits/stdc++.h>` and `using namespace std;`.

## Index

- [Lesson 1: Iterators](./Lesson_1_Iterators.md)
  - [Where do begin() and end() point?](./Lesson_1_Iterators.md#where-do-begin-and-end-point)
  - [How do you move an iterator?](./Lesson_1_Iterators.md#how-do-you-move-an-iterator)
  - [What does each container allow?](./Lesson_1_Iterators.md#what-does-each-container-allow)
  - [Cheat sheet](./Lesson_1_Iterators.md#cheat-sheet)
- [Lesson 2: Lambdas](./Lesson_2_Lambdas.md)
  - [How do you write a lambda?](./Lesson_2_Lambdas.md#how-do-you-write-a-lambda)
  - [How does a lambda see outside variables?](./Lesson_2_Lambdas.md#how-does-a-lambda-see-outside-variables)
    - [Captures](./Lesson_2_Lambdas.md#captures)
    - [Mutable](./Lesson_2_Lambdas.md#mutable)
  - [How does a lambda plug into STL algorithms?](./Lesson_2_Lambdas.md#how-does-a-lambda-plug-into-stl-algorithms)
  - [How do you write a recursive lambda?](./Lesson_2_Lambdas.md#how-do-you-write-a-recursive-lambda)
    - [Fix 1: std::function](./Lesson_2_Lambdas.md#fix-1-stdfunction)
    - [Fix 2: Pass the lambda to itself (auto self)](./Lesson_2_Lambdas.md#fix-2-pass-the-lambda-to-itself-auto-self)
