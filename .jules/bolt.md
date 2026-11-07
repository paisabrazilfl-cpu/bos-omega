## 2026-05-18 - Single-Pass String Normalization in C Governance & Routing

**Learning:** Repeated calls to `strcasestr()` in governance/routing loops perform character lowercasing on every comparison pass. Pre-normalizing input strings once into a stack buffer reduces keyword scanning time from $O(N \cdot M)$ to $O(N + M)$ and allows exact `strstr()` matching. Additionally, deferring `curl_global_init()` avoids SSL/crypto initialization overhead on local operations.

**Action:** Always pre-normalize input buffers once per scope when performing multiple case-insensitive keyword scans in performance-critical C routines.
