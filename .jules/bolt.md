## 2026-05-18 - [String Search Normalization]
**Learning:** Case-insensitive string searching in C via `strcasestr` is significantly slower than exact matching with `strstr` because it performs character conversion during every comparison. Pre-normalizing haystacks to lowercase once per scope is a high-impact optimization for multi-match scenarios.
**Action:** Always pre-normalize strings before performing multiple case-insensitive matches in performance-critical C code.
