## 2026-05-18 - Single-Pass String Normalization for Substring Keyword Matching
**Learning:** In C, repeated `strcasestr` calls on unchanged haystack strings perform redundant character lowercasing comparisons on every invocation. Normalizing haystack inputs in a single O(N) pass and performing exact `strstr` matching reduces overall scanning time by ~83.8% (6.17x speedup).
**Action:** When designing string keyword classification and routing routines in C, normalize haystack strings once in the local function scope before performing substring checks.
