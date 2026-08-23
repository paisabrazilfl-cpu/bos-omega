## 2026-05-18 - Single-Pass Normalization vs Multi-Pass `strcasestr`
**Learning:** Calling `strcasestr` repeatedly across multi-keyword decision trees (e.g., governance risk assessment & tool resolution) causes severe performance overhead because `strcasestr` performs character lowercasing conversion repeatedly on every comparison. Single-pass string normalization into a stack buffer followed by exact `strstr` matches yields a ~5.33x speedup.
**Action:** Always pre-lowercase string inputs once at function entry or runtime scope before applying sequential keyword matching routines.
