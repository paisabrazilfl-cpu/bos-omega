## 2026-05-18 - Deferring libcurl Global Initialization in CLI Runtime

**Learning:** `curl_global_init(CURL_GLOBAL_ALL)` initializes SSL libraries, crypto backends, and network subsystems. Calling `curl_global_init` unconditionally at the entry point of a multi-command CLI tool introduces unnecessary startup overhead on non-network subcommands like `lang` or invalid option parsing.

**Action:** Defer `curl_global_init` and `curl_global_cleanup` to network-active subcommands (`run` / `run_agent`) to keep lightweight subcommands fast and minimize process startup latency.
