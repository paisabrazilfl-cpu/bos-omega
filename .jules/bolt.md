# Bolt's Performance Journal

## 2025-05-14 - Initialization
**Learning:** Initialized Bolt's journal. The codebase consists of a C runtime `bos_omega_production.c`.
**Action:** I will look for other bottlenecks in `bos_omega_production.c`.

## 2025-05-14 - Optimized String Search in Risk Assessment
**Learning:** Replacing multiple `strcasestr` calls with a single pre-lowercasing pass followed by multiple `strstr` calls yields an approximately 4.9x-5.x speedup in keyword analysis. Using a fixed-size stack buffer for normalization can cause security bypasses via truncation; dynamic allocation (`malloc`) using the exact input length is required to maintain both performance and security.
**Action:** Use pre-normalized buffers for repetitive string searching on the same input, ensuring the buffer size matches the input length to avoid truncation.
