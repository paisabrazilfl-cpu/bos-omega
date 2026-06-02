## 2025-05-15 - [String Search Performance]
**Learning:** In the C runtime, multiple calls to `strcasestr` on the same input string are significantly slower than lowercasing the string once and using `strstr`. This is especially relevant in risk assessment and tool resolution paths where dozens of checks are performed.
**Action:** Always pre-normalize user input strings to lowercase if multiple case-insensitive searches are required. Use stack-allocated buffers for small to medium strings (up to `MAX_TEXT`) to avoid `malloc` overhead while maintaining performance.
