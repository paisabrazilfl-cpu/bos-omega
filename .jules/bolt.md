# Bolt's Performance Journal

Critical learnings on performance bottlenecks, unexpected optimization behaviors, and architecture patterns.

## 2026-05-18 - Pre-normalizing haystacks beats repeated case-insensitive string searching
**Learning:** `strcasestr` performs repeated case-folding on every single character comparison across multiple needle searches against the same input string. In governance assessment (`assess_risk`) and tool routing (`resolve_tool`), up to 20 case-insensitive searches were performed per task. Pre-normalizing the input string to lowercase in a single O(N) pass and using standard `strstr` reduced string scanning time from 1640.83 ms to 207.64 ms across 800,000 evaluations (an ~8x speedup or 87.3% latency reduction).
**Action:** When evaluating multiple keywords against user inputs in C, pre-normalize the string once in a stack buffer and match with exact `strstr` rather than issuing repeated `strcasestr` calls.
