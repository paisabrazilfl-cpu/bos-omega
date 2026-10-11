## 2026-05-18 - Single O(N) Normalization vs Multi-Pass Case-Insensitive String Scanning

**Learning:** Case-insensitive string searching in C via `strcasestr` is significantly slower than exact matching with `strstr` because it performs character conversion during every comparison across multiple keyword searches. Pre-normalizing haystacks once per scope reduced keyword evaluation overhead from O(N*M) character conversions to a single O(N) pass, enabling exact O(N+M) string scanning.

**Action:** Whenever performing multiple keyword checks against an input string, normalize the input string once into a stack buffer and perform exact `strstr` matching rather than calling `strcasestr` repeatedly.
