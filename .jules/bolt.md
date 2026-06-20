## 2025-05-15 - strcasestr O(N*M) conversion overhead
**Learning:** In scenarios with multiple keyword searches on a large haystack (e.g., 16KB), `strcasestr` is significantly slower than `strstr` because it performs character conversion for every comparison.
**Action:** Pre-normalize the haystack to lowercase once per scope and use `strstr` for a measured ~5.7x speedup.
