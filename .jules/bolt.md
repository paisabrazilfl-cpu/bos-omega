## 2026-05-18 - Deferring libcurl Global Initialization
**Learning:** `curl_global_init()` allocates global SSL/TLS, crypto, and DNS resources which takes ~0.95ms per process invocation in C CLI tools. Deferring libcurl initialization to network-active subcommands eliminates this startup latency for non-network subcommands like `lang`.
**Action:** Always defer network global initializations (like libcurl or OpenSSL) to specific network-dependent execution branches in CLI binaries.
