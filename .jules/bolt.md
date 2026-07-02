## 2025-05-15 - strcasestr vs strstr pre-normalization overhead
**Learning:** In a C environment (Ubuntu Noble), strcasestr is significantly slower (~5.3x) than strstr when searching for multiple keywords in the same haystack. This is because strcasestr performs case conversion on every comparison. Pre-normalizing the haystack once into a stack buffer and using strstr provides a massive speedup for multi-keyword matching.
**Action:** Always pre-normalize haystacks to lowercase when performing multiple case-insensitive searches in a single scope.

## 2025-05-15 - Redundant formatting in retry loops
**Learning:** Hoisting constant snprintf/formatting operations out of retry loops prevents unnecessary CPU cycles and memory operations.
**Action:** Check retry loops for invariant string formatting and move them outside the loop.
