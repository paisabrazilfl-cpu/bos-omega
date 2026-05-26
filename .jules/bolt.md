## 2024-05-26 - [String Search Optimization in C Runtime]
**Learning:** Multiple calls to `strcasestr` on the same input string are inefficient due to repeated case-folding. Pre-lowercasing the input once into a temporary buffer and using `strstr` with lowercase needles significantly improves performance.
**Action:** Always prefer pre-lowercasing inputs for multi-pattern matching in C to achieve measurable speedups (~4x in this case).
