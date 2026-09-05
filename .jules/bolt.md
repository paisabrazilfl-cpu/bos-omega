## 2025-05-18 - Pre-normalizing multi-match string scanning
**Learning:** `strcasestr` repeats character lowercasing for every character comparison across all candidate keywords, yielding O(N*M) behavior. Normalizing haystack strings once into a stack buffer reduces keyword scanning to fast standard `strstr` operations.
**Action:** Always pre-normalize input buffers in multi-match string classification loops.
