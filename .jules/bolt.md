# Bolt's Performance Journal

## 2026-05-18 - Case-Insensitive Search Optimization in C
**Learning:** `strcasestr` performs character conversion on every comparison, resulting in a high O(N*M) overhead when checking multiple patterns against a single string. Pre-normalizing the haystack once per scope avoids this redundant overhead, allowing us to use exact `strstr` matching which is significantly faster.
**Action:** Pre-normalize input haystacks to lowercase once before multi-match operations, and then use exact `strstr` or custom boundary word matchers like `has_word`.
