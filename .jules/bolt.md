## 2025-05-18 - Single-Pass String Normalization for Governance and Routing

**Learning:** Repeated `strcasestr` calls across governance and routing routines scale poorly because every comparison converts characters on-the-fly. Normalizing haystack strings to lowercase in a single $O(N)$ pass upfront allows exact $O(N+M)$ `strstr` comparisons, reducing string scanning latency significantly without adding external dependencies or compromising memory safety.
**Action:** Always pre-normalize haystacks before multi-keyword searches in high-frequency routing and classification paths.
