# Bolt's Performance Journal ⚡

## 2026-05-18 - Case-insensitive String Matching Overhead in C
**Learning:** Case-insensitive string searching in C via `strcasestr` is significantly slower than exact matching with `strstr` because it performs character conversion during every comparison. Pre-normalizing haystacks once per scope is a high-impact optimization for multi-match scenarios, reducing time complexity from O(N*M) to O(N+M).
**Action:** Replace multiple `strcasestr` calls with a single `normalize` pass (converting the input to lowercase once) and then use standard `strstr` for fast keyword matching. Ensure the `normalize` function is robust against NULL inputs and buffer overflows.
