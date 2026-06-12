# Bolt's Journal - Critical Learnings

## 2025-05-22 - Pre-normalization String Search Optimization
**Learning:** Case-insensitive string searching in C via `strcasestr` is significantly slower than exact matching with `strstr` because it performs character conversion during every comparison. Pre-normalizing haystacks to lowercase once per scope is a high-impact optimization for multi-match scenarios.
**Action:** Always pre-normalize haystacks to lowercase when performing multiple case-insensitive keyword checks on the same string.
