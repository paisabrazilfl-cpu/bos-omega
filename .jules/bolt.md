## 2026-05-18 - Case-Insensitive String Pre-normalization
**Learning:** In C, repeated multi-keyword string searching using `strcasestr` can become a performance bottleneck due to character conversions at each comparison level. Converting the haystack once into a stack-allocated lowercase buffer (`O(N)`) and subsequently calling `strstr` (which is highly optimized and matches exactly, `O(M)`) provides a ~3x speedup.
**Action:** Pre-normalize input strings once per function scope when doing multi-keyword classification/routing to optimize string searching and reduce computational overhead.
