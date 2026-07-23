# Bolt's Performance Journal

## 2026-05-18 - Case-Insensitive String Search Optimization in C
**Learning:** Case-insensitive string searching in C via `strcasestr` is significantly slower than exact matching with `strstr` because it performs character conversion during every comparison. Pre-normalizing haystacks once per scope (e.g., converting the entire input to lowercase) is a high-impact optimization for multi-match scenarios, reducing time complexity from O(N*M) character conversions to a single O(N) pass followed by O(M) fast exact comparisons.
**Action:** Replace multi-match `strcasestr` calls with a single `normalize` call that lowercases the input into a stack-allocated buffer (with bounds checks to prevent overflows), then perform matching using standard `strstr` with pre-lowercased needles.
