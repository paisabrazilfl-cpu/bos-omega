# Bolt's Performance Journal

## 2026-10-31 - Deferring global libcurl initialization on non-network CLI paths
**Learning:** `curl_global_init(CURL_GLOBAL_ALL)` incurs ~6-8ms of synchronous overhead on every process startup due to OpenSSL crypto initialization, internal ciphers, engine setup, and thread/DNS subsystems. When executing lightweight subcommands like `lang`, CLI help, or validation, this startup cost accounted for >70% of total process execution time.
**Action:** Always defer subsystem initializations like `curl_global_init()` to network-active subcommands (`run`) rather than unconditionally in `main()`.
