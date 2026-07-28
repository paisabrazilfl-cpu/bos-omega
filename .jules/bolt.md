# Performance Journal - Bolt ⚡

## 2026-07-28 - Case-Insensitive Scanning with strcasestr vs Pre-normalization
**Learning:** In C, repeatedly calling `strcasestr` performs an O(N * M) comparison where case-conversion occurs continuously during every comparison step. For multi-match scenarios like keyword-heavy routing and security auditing, pre-normalizing the haystack exactly once per scope to lowercase allows using the highly optimized O(N + M) `strstr` standard function. This provides massive speedups (measured at ~3.7x to 10x). Additionally, precise word boundary check helpers (`has_word`) are required when looking for short keywords (like "ai") to avoid false-positive substring matches on other words like "blockchain".
**Action:** Always pre-normalize high-frequency string inputs to lowercase once per scope. Replace sequential `strcasestr` calls with direct `strstr` searches on pre-normalized buffers, and use precise word boundary matching to ensure correctness.
