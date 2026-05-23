## 2025-05-23 - [Optimized string search in C runtime]
**Learning:** In the C runtime, functions like `assess_risk`, `resolve_tool`, and `run_lang` were using `strcasestr` multiple times on the same input string. This was inefficient because `strcasestr` has to handle case-insensitivity for every search.
**Action:** By lowercasing the input string once into a local buffer and using `strstr` with lowercase needles, we achieve a significant performance boost (~4x based on benchmarks) for these heuristic-based routing and risk assessment functions.
