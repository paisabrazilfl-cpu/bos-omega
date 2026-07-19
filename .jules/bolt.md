# Performance Journal

## 2026-05-18 - Case-insensitive String Searching Optimization
**Learning:** Case-insensitive string searching in C via `strcasestr` is significantly slower than exact matching with `strstr` because it performs character conversion during every comparison. Pre-normalizing haystacks once per scope is a high-impact optimization for multi-match scenarios, reducing time complexity from O(N*M) with redundant allocations/conversions to a single O(N) normalization pass followed by O(M) `strstr` matches.
**Action:** Replace multiple `strcasestr` calls with a single lowercase pass using `normalize()` and subsequent `strstr` calls.
