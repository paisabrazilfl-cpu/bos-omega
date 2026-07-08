## 2025-05-15 - strcasestr O(N*M) conversion overhead
**Learning:** `strcasestr` in C is significantly slower than `strstr` for multiple keyword matches because it performs character conversion and locale checks during every comparison. Pre-normalizing the haystack once and using `strstr` with pre-lowercased needles provides a 2.6x to 18x speedup depending on string length and number of patterns.
**Action:** Always pre-normalize haystacks when performing more than 2-3 case-insensitive substring searches within the same scope.
