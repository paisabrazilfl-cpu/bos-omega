## 2025-05-14 - Pre-normalized string search vs strcasestr
**Learning:** Using `strcasestr` for multiple keyword matches on the same large buffer (16KB) is highly inefficient because it performs case conversion for every byte comparison in every search. Pre-normalizing the buffer to lowercase once and using `strstr` yielded a measured 8.10x speedup (53.47s vs 6.60s for 100k iterations).
**Action:** When performing more than 2-3 case-insensitive searches on the same large string, pre-normalize the string once and use exact match searching.
