# Bolt's Performance Journal ⚡

## 2026-05-18 - Replacing strcasestr with Pre-normalized strstr
**Learning:** Case-insensitive string searching in C via `strcasestr` is significantly slower than exact matching with `strstr` because it performs character conversion during every comparison. Pre-normalizing haystacks once per scope is a high-impact optimization for multi-match scenarios, reducing time complexity from O(N*M) to O(N+M).
**Action:** Replace multiple `strcasestr` calls with a single lowercase normalization pass using a stack-allocated buffer, then use `strstr` or `has_word` for keyword matching.
