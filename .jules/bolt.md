# Bolt's Performance Journal

## 2025-02-23 - Pre-normalization string search optimization
**Learning:** `strcasestr` performs lowercasing conversions on every comparison O(N*M). Pre-normalizing haystack strings into lowercase once per function invocation reduces search complexity to O(N+M) with `strstr`.
**Action:** Always pre-normalize input buffers in `bos_omega_production.c` before running multiple string keyword searches.
