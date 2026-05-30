## 2026-05-30 - [Efficient String Search in C]
**Learning:** Using `strcasestr` repeatedly on the same large buffer (16KB) in a tight loop is extremely inefficient because it performs case-folding on every call. Pre-lowercasing the buffer once into a local stack-allocated array and then using `strstr` (case-sensitive) provides a ~5.3x speedup.
**Action:** Always pre-process strings once if they are to be searched multiple times with case-insensitivity, but ensure bounds safety when copying into local buffers.
