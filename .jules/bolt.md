## 2026-03-29 - Lazy Initialization of Network Libraries (libcurl)

**Learning:**
Calling `curl_global_init(CURL_GLOBAL_ALL)` indiscriminately on application startup introduces unnecessary overhead (~1.14ms) from initializing SSL/crypto modules, DNS, and internal libcurl structures, even when executing non-network subcommands (e.g., `lang`) or handling invalid CLI arguments.

**Action:**
Defer `curl_global_init` using a static boolean guard to the precise network execution path (`dispatch_http`), and register cleanup with `atexit(curl_global_cleanup)` to ensure zero startup overhead for local/offline subcommands.
