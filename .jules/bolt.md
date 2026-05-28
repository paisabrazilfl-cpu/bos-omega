## 2026-05-28 - [String Normalization & Search Optimization]
**Learning:** `strcasestr` is significantly slower than `strstr` because it performs case folding on every comparison. Normalizing the haystack once and using `strstr` yields a ~3.4x - 5x speedup in keyword-heavy paths. Additionally, stack allocation is preferred over heap allocation for small, fixed-size buffers in hot paths to avoid `malloc` overhead and potential null pointer failures.
**Action:** Always pre-normalize strings before multiple search passes and prefer stack buffers when the maximum size is known and reasonable.
