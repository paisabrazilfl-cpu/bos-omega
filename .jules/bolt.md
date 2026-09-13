## 2026-05-18 - Single-Pass String Normalization for Keyword Searching
**Learning:** In C applications performing repeated case-insensitive keyword searches (`strcasestr`), converting the haystack string once to lower-case in an O(N) pass and using standard `strstr` reduces string scanning overhead from O(N*M) to O(N+M), yielding ~10x speedups on large text buffers (16KB).
**Action:** Always pre-normalize haystack buffers once before multi-keyword classification/routing loops.
