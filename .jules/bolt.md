# Bolt's Performance Journal

## 2026-05-18 - Case-insensitive string search overhead with strcasestr
**Learning:** `strcasestr` performs a case conversion during every character comparison, leading to O(N*M) conversion overhead. When doing multiple keyword matches, pre-normalizing (lowercasing) the haystack once per scope to perform exact string searches with `strstr` is much faster (O(N) normalization + O(M) search).
**Action:** Replace multiple `strcasestr` calls with a single lowercase normalization pass using a safe stack-allocated buffer, followed by `strstr` calls on the normalized buffer.
