## 2026-05-18 - String Case Normalization Optimization
**Learning:** O(N*M) scanning complexity via `strcasestr` can be optimized to O(N+M) by performing a single lowercase pre-normalization pass on the input string, allowing faster `strstr` matching.
**Action:** Always pre-normalize Haystacks to lowercase before scanning them with multiple keyword needles.
