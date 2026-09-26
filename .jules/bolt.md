## 2026-05-18 - Deferred cURL Initialization and Payload Buffer Sizing

**Learning:** Deferring `curl_global_init` to network-active CLI execution paths prevents unnecessary SSL, socket, and crypto initialization on non-network operations. Additionally, sizing payload buffers with headroom (`MAX_TEXT + 128`) and using string precision specifiers (`%.*s`) prevents compiler truncation warnings when formatting JSON payloads around maximum length inputs.
**Action:** Always scope global library initialization to commands that perform network requests and provide explicit buffer headroom and precision specifiers when wrapping large dynamic strings in JSON.
