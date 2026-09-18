# Bolt's Journal - Critical Learnings

## 2026-09-18 - Profiling C Runtime Performance Limits
**Learning:** Profiling `bos_omega_production.c` showed total process execution time is ~2-13ms, with string searching (`str_has`/`strcasestr`) taking under 2 microseconds across 1,000,000 iterations. Shell execution (`popen`) and network I/O (`curl_easy_perform`) dominate total runtime. Micro-optimizing string operations or stack allocations on cold paths provides no measurable performance improvement.
**Action:** Respect "measure first, optimize second". Avoid micro-optimizing cold paths with unmeasurable gains and halt PR creation when no genuine bottleneck exists.
