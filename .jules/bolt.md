## 2026-06-24 - String search optimization in C

**Learning:** Replacing `strcasestr` with a single `str_tolower` normalization pass followed by multiple `strstr` calls yields a significant performance boost (measured ~6.3x speedup on 16KB haystacks with 27 needles). `strcasestr` is slow because it performs character conversion during every comparison.

**Action:** For performance-critical paths involving multiple keyword searches on the same string, normalize the haystack to lowercase once and use `strstr`. Ensure null-safety and buffer bounds during normalization.
