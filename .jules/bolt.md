## 2026-05-18 - Case-insensitive searching O(N*M) conversion overhead in C runtime

**Learning:** Case-insensitive string searching via `strcasestr` performs implicit locale-dependent character conversions on the fly during every comparison step. In multi-keyword matching loops (such as the risk assessment and tool resolution filters), this leads to a costly O(N*M) scanning complexity where characters are repeatedly lowercase-converted. Pre-normalizing the haystack exactly once per scope to lowercase reduces character scanning to O(N + M) and allows high-performance exact matching via `strstr`.

**Action:** Replace direct `strcasestr` matches in `assess_risk`, `resolve_tool`, and language advisor logic with a single `normalize` pass on the input string, followed by `strstr` matching. Ensure the `normalize` function handles NULL source pointers safely, is bound-checked to prevent buffer overflow, and casts to `(unsigned char)` before converting.
