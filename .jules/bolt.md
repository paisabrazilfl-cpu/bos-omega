## 2025-07-16 - Pre-lowercased string search optimization
**Learning:** In C, multiple `strcasestr` calls on the same haystack are significantly slower than a single O(N) normalization pass followed by multiple O(M) `strstr` matches. This is because `strcasestr` performs character conversion during every comparison.
**Action:** Pre-normalize inputs to lowercase when performing multiple keyword searches in the governance and routing engines.
