## 2026-05-18 - Deferring Libcurl Global Initialization for Non-Network Subcommands

**Learning:** `curl_global_init(CURL_GLOBAL_ALL)` initializes SSL libraries, DNS resolvers, and memory allocations on binary startup. Executing `curl_global_init` unconditionally on process entry penalizes non-network CLI subcommands (like `lang` recommendations or help messages) with unnecessary setup and cleanup overhead.
**Action:** Defer `curl_global_init` and `curl_global_cleanup` to the specific execution path where HTTP requests are dispatched, keeping local utility commands lean and fast.
