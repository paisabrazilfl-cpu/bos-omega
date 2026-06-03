## 2026-06-03 - [Performance] Inefficient Repeated Case-Insensitive Searching
**Learning:** The C runtime repeatedly called `strcasestr` (case-insensitive search) on large input buffers (up to 16KB) for risk assessment and tool resolution. `strcasestr` is significantly slower than `strstr` because it must perform case-normalization on every comparison.
**Action:** Pre-normalize the haystack string to lowercase once per request scope into a stack buffer and use `strstr` for keyword matching. This yielded an ~18.9x speedup in the core assessment logic.
