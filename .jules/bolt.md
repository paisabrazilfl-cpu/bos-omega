## 2025-05-18 - Single-Pass String Normalization in Governance Routines
**Learning:** Pre-normalizing input strings in a single O(N) pass and using exact `strstr` instead of repeated `strcasestr` calls in governance/routing routines reduced keyword scanning time by 64% (2.78x speedup).
**Action:** When a string is scanned repeatedly against multiple keywords in C, normalize it once per scope to avoid redundant per-character conversions in `strcasestr`.
