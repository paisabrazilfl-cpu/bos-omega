## 2025-05-15 - strcasestr Overhead in Multi-Match Scenarios
**Learning:** `strcasestr` is significantly slower than `strstr` because it performs character conversion during every comparison. In scenarios where a large haystack is checked against many needles, pre-normalizing the haystack once and using `strstr` provides a massive performance boost.
**Action:** Always pre-normalize the haystack to lowercase once per scope when performing multiple case-insensitive searches, and use `strstr` for the actual matching.

## 2025-05-15 - Unsafe String Normalization Utility
**Learning:** A string normalization helper like `str_tolower` must always ensure null-termination, even if the input is NULL or empty. Relying on the loop to null-terminate is dangerous if the loop doesn't execute.
**Action:** Proactively null-terminate the destination buffer (`dest[0] = '\0'`) at the start of the utility function.
