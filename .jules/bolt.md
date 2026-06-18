## 2025-05-15 - String normalization for multi-keyword search
**Learning:** `strcasestr()` is significantly slower than `strstr()` because it performs character conversion for every comparison. When checking multiple keywords against a large haystack (16KB), pre-normalizing the haystack once and using `strstr()` provides a massive performance boost (~19x speedup).
**Action:** Always pre-normalize input strings if multiple case-insensitive keyword searches are required on the same buffer.
