## 2026-05-18 - Single-Pass Lowercasing vs Repeated Case-Insensitive Matching

**Learning:** `strcasestr` performs per-character case conversions dynamically across the haystack for every search keyword. In functions performing multiple keyword checks against the same string (e.g. risk assessment and tool resolution), replacing `strcasestr` calls with a single `tolower` normalization pass and exact `strstr` matching reduces overall string matching time from ~3.24s to ~0.54s for 1M invocations (~6x speedup).

**Action:** When performing multi-keyword scanning on string buffers in C, normalize the buffer once per function scope and use exact `strstr` rather than multiple `strcasestr` calls.
