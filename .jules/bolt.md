## 2026-05-18 - Single-Pass String Normalization vs Repeated strcasestr

**Learning:** `strcasestr()` converts characters dynamically on every character comparison across every keyword check. In routines like governance (`assess_risk`) and routing (`resolve_tool`) that execute up to 18 keyword checks per string, converting input characters once in a single O(N) pass and using exact `strstr()` yields a 6.41x speedup (84.4% reduction in string scanning time).
**Action:** When scanning input text against multiple keyword patterns, always pre-normalize the input string once per scope into a stack buffer before matching with exact `strstr()`.
