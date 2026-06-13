## 2025-05-14 - Optimized String Search via Pre-normalization
**Learning:** `strcasestr` is significantly slower than `strstr` on long haystacks (16KB) because it performs case conversion on every character comparison for every search. Pre-normalizing the haystack once and using `strstr` yields a ~3.1x speedup in this codebase.
**Action:** Always pre-normalize buffers when performing multiple case-insensitive searches on the same large dataset.
