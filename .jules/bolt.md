## 2024-05-20 - [Optimized String Search and Runtime Operations]
**Learning:** strcasestr is significantly slower than strstr because it performs case conversion on every comparison. For multi-needle searches on a fixed haystack, lowercasing the haystack once and using strstr is much more efficient.
**Action:** Always pre-normalize strings when performing multiple case-insensitive matches in high-frequency paths.
