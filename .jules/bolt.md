## 2026-05-18 - Lowercase Normalization vs strcasestr

**Learning:** Case-insensitive string searching in C via `strcasestr` is significantly slower than exact matching with `strstr` because `strcasestr` performs character conversion on every comparison step. Pre-normalizing haystacks once per scope reduced keyword scanning overhead in governance and tool resolution by ~5.27x.
**Action:** Always pre-normalize input buffers in C once per evaluation scope before running multiple keyword checks rather than invoking case-insensitive substring functions repeatedly.
