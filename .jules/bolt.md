## 2025-05-15 - String Search Optimization in C Runtime
**Learning:** Using `strcasestr` for multiple keyword searches on the same large haystack is highly inefficient because it performs case conversion on every character comparison for every search. Pre-lowercasing the haystack once and using `strstr` provides a massive performance boost.
**Action:** Always pre-normalize strings when performing multiple case-insensitive searches in C.
