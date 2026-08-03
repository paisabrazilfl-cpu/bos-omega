# Bolt's Performance Journal

## 2026-05-18 - Case-Insensitive String Normalization
**Learning:** Case-insensitive string searching via `strcasestr` is significantly slower than exact matching with `strstr` because it performs character conversion during every comparison. Pre-normalizing haystacks once per scope to lowercase reduces keyword scanning complexity from O(N*M) to O(N+M) and is a high-impact optimization for multi-match scenarios.
**Action:** Always pre-normalize user inputs (haystacks) using a single O(N) lowercase sweep before running multiple substring matches with O(M) `strstr` or boundary-safe word checks.
