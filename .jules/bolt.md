## 2026-05-18 - Case-Insensitive String Matching Optimization
**Learning:** Performing multiple case-insensitive scans using `strcasestr` can be an O(N*M) operation, which adds up on large inputs or frequent checks. Pre-normalizing haystacks to lowercase once per scope enables exact matches using the much faster `strstr`, reducing time complexity.
**Action:** Use pre-normalized lowercasing on target buffers for multi-match routines.
