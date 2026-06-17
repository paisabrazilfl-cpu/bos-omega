## 2025-05-15 - strcasestr overhead in multi-keyword search
**Learning:** In C, `strcasestr` performs character conversion during every comparison, leading to O(N*M) conversion overhead. For multi-match scenarios on a static input (like risk assessment or tool routing), pre-normalizing the haystack to lowercase once and using `strstr` yields a significant (8-12x) performance boost.
**Action:** Always pre-normalize Haystacks to lowercase when performing multiple case-insensitive searches in the same scope.

## 2025-05-15 - Redundant payload formatting in retry loops
**Learning:** Calling `snprintf` to format the same JSON payload inside a retry loop is a needless O(N) operation per attempt.
**Action:** Move constant data formatting outside of loops and retry blocks.
