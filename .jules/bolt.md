## 2026-05-19 - [Optimizing case-insensitive search in C]
**Learning:** `strcasestr` (a GNU extension) is often significantly slower than `strstr` because it performs case conversion logic for every character on every call. In performance-critical paths where multiple keywords are searched against the same large text (e.g., a 16KB objective buffer), it is measurably faster (up to 80x) to lowercase the haystack once into a temporary buffer and then use `strstr`.

**Action:** When performing multiple case-insensitive string matches on a single source, always pre-lowercase the source string into a local buffer and use case-sensitive matching functions. Always use bounds-checking when copying to the lowercased buffer to avoid security vulnerabilities.
