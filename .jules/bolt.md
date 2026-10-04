## 2026-05-18 - Pre-normalizing input strings for multi-keyword searches

**Learning:** `strcasestr()` converts characters to lowercase repeatedly during every comparison. When matching an input string against multiple keyword patterns in governance and routing routines, pre-normalizing the string once to lowercase and performing exact `strstr()` searches reduces string scanning complexity and eliminates redundant character conversion overhead.

**Action:** Always pre-normalize input strings in single O(N) pass when performing multiple case-insensitive pattern checks across governance or decision routines.
