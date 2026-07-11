## 2025-05-15 - strcasestr O(N*M) conversion overhead
**Learning:** In C, `strcasestr` is significantly slower than exact matching with `strstr` because it performs character conversion during every comparison. When searching for many keywords in the same large buffer (e.g., 16KB), pre-normalizing the buffer once and using `strstr` provides a ~7x performance boost.
**Action:** Pre-normalize haystack buffers once per scope and use `strstr` with pre-lowercased needles for multi-match scenarios.
