## 2025-05-15 - Case-insensitive string search optimization
**Learning:** In C, multiple `strcasestr()` calls on the same buffer are inefficient as they perform character conversion repeatedly. Pre-normalizing the buffer to lowercase once and using `strstr()` for subsequent matches provides a significant performance boost.
**Action:** Replace multiple `strcasestr()` scans on static buffers (like `rt->objective`) with a single `normalize()` pass followed by exact `strstr()` matches. Measured a ~5.5x speedup in keyword-heavy search paths (Governance and Tool engines).

## 2025-05-15 - Buffer size safety for JSON payloads
**Learning:** Wrapping a large buffer (`MAX_TEXT`) in a JSON object with `snprintf` requires additional headroom in the destination buffer to avoid truncation warnings and potential data loss.
**Action:** Use `char payload[MAX_TEXT + 128]` and `%.*s` precision in `snprintf` to safely wrap `MAX_TEXT` sized strings in JSON.
