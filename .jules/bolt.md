## 2026-05-18 - Case-Insensitive O(N*M) Conversion Overhead in strcasestr
**Learning:** Case-insensitive string searching in C via `strcasestr` is significantly slower than exact matching with `strstr` because it performs character conversion during every comparison. Pre-normalizing haystacks once per scope is a high-impact optimization for multi-match scenarios.
**Action:** Always pre-normalize input haystacks to lowercase once when starting an execution path and use `strstr` with exact lowercase needle strings for keyword-heavy routing, governance, or parsing engines.
