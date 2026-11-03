# Bolt's Performance Journal

## 2025-05-18 - Single-Pass Normalization vs. Repeated strcasestr in Governance Engine
**Learning:** `strcasestr` is convenient for case-insensitive string matching, but when a task string is evaluated across dozens of risk keywords and tool routing patterns, repeatedly scanning and case-folding characters inside `strcasestr` creates measurable overhead. Pre-normalizing the string into a lowercase buffer in a single O(N) pass and using exact `strstr` yielded an ~8.12x speedup (~87.7% reduction in string scanning time) across governance and routing checks.
**Action:** In C/C++ string scanning pipelines with multiple keyword rules against a shared input, always pre-normalize the haystack once per scope and perform exact matching (`strstr`) instead of calling `strcasestr` per keyword.
