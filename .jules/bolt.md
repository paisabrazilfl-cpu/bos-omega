## 2025-05-18 - Single-Pass String Normalization for Multi-Match Keyword Scanning

**Learning:** Case-insensitive string searching in C via `strcasestr` is significantly slower than exact matching with `strstr` because `strcasestr` converts characters on every single character comparison across multiple searches. Pre-normalizing haystacks to lowercase once in O(N) time and using `strstr` yields a ~5.8x speedup in multi-keyword classification tasks.

**Action:** Whenever multiple `strcasestr` queries are evaluated against the same input string (such as in governance, risk classification, or tool resolution engines), pre-normalize the string once in stack allocation or per request scope and use `strstr`.
