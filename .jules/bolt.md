## 2026-05-18 - String Search Optimization in C Runtime
**Learning:** Replacing repeated `strcasestr` calls with a single `str_tolower` normalization followed by multiple `strstr` calls provides a significant performance boost (~4-5x speedup) in command routing and risk assessment logic.
**Action:** Use pre-lowercased buffers for multiple substring searches on the same input in performance-critical C paths.

**Learning:** `str_tolower` must handle NULL or empty sources explicitly and ensure null-termination to avoid undefined behavior when passed to `strstr`.
**Action:** Always verify string normalization helpers for edge cases like empty strings and proper null-termination.
