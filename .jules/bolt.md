## 2026-05-18 - Single O(N) String Normalization vs Repeated `strcasestr` Scanning

**Learning:** In C applications performing multi-keyword governance and routing checks, repeatedly invoking `strcasestr` on non-normalized input strings forces case conversions on every substring lookup attempt. Pre-normalizing input strings into lowercase in a single O(N) pass and using exact `strstr` matching reduces scanning overhead significantly without affecting classification outcome.

**Action:** Whenever a runtime string needs to be matched against multiple constant keywords, pre-normalize the string once in a stack buffer or during initialization, and use exact `strstr` or `has_word` comparisons instead of repeated `strcasestr`.
