# Bolt's Performance Journal

## 2025-06-14 - String Search Optimization in C
**Learning:** Case-insensitive string searching using `strcasestr` in C is significantly slower than pre-normalizing the input to lowercase and using `strstr`. In scenarios where multiple keywords (needles) are checked against the same input (haystack), normalizing once per scope yields a ~3x to 6x speedup.
**Action:** Always pre-normalize the haystack to lowercase if performing more than 2-3 case-insensitive searches on the same string in a single function scope.

## 2025-06-14 - Redundant snprintf in Retry Loops
**Learning:** Formatting strings with `snprintf` inside a retry loop where the input doesn't change is a wasted computation.
**Action:** Always hoist constant string formatting operations outside of loops to minimize CPU cycles per iteration.
