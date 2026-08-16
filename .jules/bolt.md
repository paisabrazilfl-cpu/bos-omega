## 2026-05-18 - Pre-normalizing Haystack Strings for Case-Insensitive Matching

**Learning:** `strcasestr` performs character conversion during every comparison on every needle evaluation. Across multiple `strcasestr` calls in governance risk assessment and tool resolution routines, this results in repeated O(N*M) conversion overhead. Normalizing input haystacks once in O(N) into a stack buffer and using `strstr` yields a ~5.6x speedup.
**Action:** When performing multi-keyword case-insensitive matching on string parameters, normalize the input string once into a lowercase stack buffer before performing exact string matches using `strstr`.
