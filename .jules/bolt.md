## 2026-05-18 - Pre-lowercasing string buffers for O(N) keyword matching
**Learning:** `strcasestr` performs character lowercasing on every comparison iteration across multiple keyword searches, resulting in O(N*M) redundant work. Normalizing input strings to lowercase once per scope reduces total scan complexity to O(N+M) with exact `strstr` matching.
**Action:** When performing multi-keyword detection or classification on text inputs in C, normalize the buffer once into a local stack buffer before scanning with `strstr`.
