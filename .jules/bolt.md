# Bolt's Performance Journal

## 2026-11-02 - Defer libcurl Global Initialization on Offline CLI Subcommands
**Learning:** `curl_global_init(CURL_GLOBAL_ALL)` eagerly initializes OpenSSL crypto ciphers, SSL algorithms, and platform DNS resolvers, consuming ~0.95ms of CPU time and substantial memory subsystem setup per process invocation. Unconditionally calling it at the start of `main()` in a multi-command CLI runtime penalizes offline/local subcommands like `lang` or argument syntax validation.
**Action:** Always lazily defer `curl_global_init()` and `curl_global_cleanup()` to network-active subcommand branches (`run`) to eliminate unnecessary SSL/DNS startup latency on offline CLI paths.
