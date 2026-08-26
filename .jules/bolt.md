## 2026-08-26 - Case-insensitive string search overhead in C governance engine
**Learning:** `strcasestr` performs character conversion on every comparison in O(N*M) time. Normalizing the haystack string once per scope reduces keyword lookup time by up to ~7.8x.
**Action:** Pre-normalize inputs using stack-allocated buffers and fast `strstr()` exact matches when performing multi-keyword scanning in runtime classification functions.
