## 2025-05-22 - [Optimizing String Search in C]
**Learning:** `strcasestr` is significantly slower than `strstr` because it performs case-folding on every comparison. When multiple patterns need to be matched against the same haystack, pre-lowercasing the haystack once and then using `strstr` provides a substantial performance boost (approx. 3.8x in this codebase).
**Action:** Use pre-normalized buffers for multi-pattern string matching in performance-critical paths.

## 2025-05-22 - [Binary Artifacts in PRs]
**Learning:** Committing compiled binaries or test logs to a source repository is an anti-pattern that bloats the repository and complicates code review.
**Action:** Always verify `.gitignore` covers build artifacts and ensure a clean environment before submission.
