## 2025-05-18 - Lowercase Normalization Before String Matching
**Learning:** Using `strcasestr` repeatedly in multi-match functions (e.g. `assess_risk` and `resolve_tool`) performs case conversions on every character comparison per keyword search. Pre-normalizing input text into a stack buffer once in O(N) time enables fast O(M) `strstr` matches.
**Action:** Always normalize string input to lowercase at the scope boundary before performing multiple case-insensitive keyword scans.
