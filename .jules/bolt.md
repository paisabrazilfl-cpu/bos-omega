# Bolt's Performance Journal

This journal documents Bolt's performance measurements, optimizations, surprises, and critical learnings in the BOS-OMEGA repository.

## 2026-05-18 - String normalization and search optimization
- **Learning:** Exact case-sensitive `strstr` on pre-normalized lowercased buffers is significantly faster than repeated `strcasestr` searches on a raw buffer, reducing complexity from O(N*M) to O(N+M) where N is the length of the string and M is the number of searches. Empirically, this optimization achieved an ~8.04x speedup under the benchmark run of 500,000 iterations across multiple inputs.
- **Action:** Implement a custom non-allocating stack `normalize` helper and refactor key functions to match pre-lowercased string buffers.
