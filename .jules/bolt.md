## 2025-05-15 - strcasestr vs Normalized strstr
**Learning:** `strcasestr` in Glibc performs character conversion on every comparison, leading to O(N*M) conversion overhead. For multi-keyword matching against a large haystack (e.g., 16KB), pre-normalizing the haystack once into a buffer and using `strstr` yields a 5x-15x speedup. However, stack-allocating large buffers (16KB) for normalization is a risk; heap allocation (strdup) is safer for production.
**Action:** Always pre-normalize haystacks once before multiple keyword searches. Use heap allocation for normalization buffers if haystack size is significant or unknown.

## 2025-05-15 - Governance Truncation Risk
**Learning:** Fixed-size normalization buffers in risk assessment engines can lead to governance bypass if security-critical keywords appear after the buffer limit.
**Action:** Use dynamically allocated normalization buffers (e.g., via `strdup`) that match the input string's full length to ensure exhaustive security scanning.
