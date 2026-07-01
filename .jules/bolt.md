## 2026-07-01 - Baseline for Keyword Search
**Learning:** Found that `strcasestr` takes ~5.18s for 100,000 iterations on a 16KB haystack with 13 needles.
**Action:** Replace `strcasestr` with `strstr` on a pre-normalized (lowercase) string to eliminate redundant case conversions.
## 2026-07-01 - Pre-normalized Keyword Search Speedup
**Learning:** Pre-normalizing a 16KB haystack once and using `strstr` yielded a ~15.8x speedup (5.18s -> 0.328s) for 100,000 iterations of risk and tool assessment.
**Action:** Always prefer one-time normalization over repeated case-insensitive searches in high-frequency logic paths.
