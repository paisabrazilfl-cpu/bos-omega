## 2026-05-18 - Case-Insensitive String Matching Optimization
**Learning:** Case-insensitive string searching in C via `strcasestr` is significantly slower than exact matching with `strstr` because it performs character conversion during every comparison. Pre-normalizing haystacks once per scope is a high-impact optimization for multi-match scenarios.
**Action:** Replace multiple `strcasestr` calls by pre-normalizing the haystack string once to lowercase using a stack-allocated buffer, then performing fast exact matching using `strstr`.
