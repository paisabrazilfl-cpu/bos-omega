# Bolt's Performance Journal

## 2025-05-15 - strcasestr Overhead in Multi-Match Scenarios
**Learning:** `strcasestr` performs character conversion on every comparison, leading to significant overhead when searching for many needles in the same haystack. Pre-normalizing the haystack to lowercase once per scope and using `strstr` yielded a ~6.4x speedup in `bos_omega_production.c`.
**Action:** Always pre-normalize haystacks when performing multiple case-insensitive searches in C to avoid redundant conversion.
