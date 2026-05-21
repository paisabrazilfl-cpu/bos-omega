## 2025-05-21 - String Search Optimization in C Runtime
**Learning:** Frequent use of `strcasestr` on the same large buffer is inefficient as it repeatedly performs case-insensitive comparisons. Pre-lowercasing the haystack once and using `strstr` with lowercased needles provides a significant performance boost (~3.3x in benchmarks).
**Action:** Always consider pre-normalizing input strings when multiple case-insensitive searches are required on the same data.
