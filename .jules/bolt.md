# Bolt's Performance Journal

## 2025-05-10 - O(N*M) String Search Overhead in Governance & Skill Resolution
**Learning:** Calling `strcasestr()` repeatedly over identical strings performs redundant character lowercasing conversions on every search check, increasing runtime complexity significantly in multi-keyword search loops. Pre-lowercasing the haystack into a temporary buffer once and using exact string matching (`strstr()`) yields ~7-8x faster classification.
**Action:** When performing multi-keyword string classifications, pre-normalize input strings into lowercase once before passing them to matching functions.
