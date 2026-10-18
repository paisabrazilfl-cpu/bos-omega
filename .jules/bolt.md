## 2025-05-20 - Pre-normalization and strstr matching in C routing engines

**Learning:** Replacing repeated `strcasestr` calls across multiple keyword checks with a single O(N) lower-case normalization pass and exact `strstr` matching reduced keyword scanning runtime by ~3.57x. Additionally, deferring `curl_global_init` for non-network subcommands avoids SSL and crypto initialization overhead during non-network operations like language advisor queries.
**Action:** Always pre-normalize input strings before multi-keyword evaluations in C runtime routing logic to avoid re-evaluating `tolower` on every comparison byte.
