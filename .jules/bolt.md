# Bolt's Performance Journal

## 2026-05-18 - Case-Insensitive Scanning with Pre-Normalized Buffers
**Learning:** `strcasestr` is significantly slower than exact matching with `strstr` because it performs character conversion during every comparison. Pre-normalizing haystacks once per scope is a high-impact optimization for multi-match scenarios.
**Action:** Pre-normalize the task or goal string into a local buffer using a single character conversion pass, then perform exact `strstr` matching.
