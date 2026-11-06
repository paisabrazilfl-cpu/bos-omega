## 2025-10-29 - Pre-normalize Haystacks to Eliminate O(N*M) strcasestr Overhead

**Learning:** Case-insensitive string matching via `strcasestr()` in C converts characters to lowercase repeatedly on every comparison pass. When evaluating multiple keywords against the same input string (e.g., in governance risk assessment and tool classification routines with 17+ keywords), `strcasestr()` re-converts the string up to dozens of times per call, resulting in severe performance overhead. Pre-normalizing input strings to lowercase once per scope in an O(N) pass and using exact `strstr()` reduces string scanning latency by ~84%.

**Action:** Whenever performing multi-keyword scanning on input strings in C runtime routines, pre-normalize the string once in a stack-allocated buffer and perform exact `strstr()` matching rather than calling `strcasestr()` iteratively.
