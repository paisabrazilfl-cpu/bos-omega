## 2025-05-15 - String Normalization for Rule Engines
**Learning:** Replacing multiple `strcasestr` calls with a single pre-normalization pass using `str_tolower` and then `strstr` yields significant performance gains (up to 6.6x) for rule-based systems with many needles, especially on larger haystacks (16KB+).
**Action:** Always pre-normalize input strings once per scope when performing multiple case-insensitive keyword searches.
