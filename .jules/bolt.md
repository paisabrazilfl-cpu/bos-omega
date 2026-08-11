# Bolt Performance Journal

## 2026-05-18 - Replacing strcasestr with Pre-normalized strstr in Heuristic Categorization
**Learning:** Case-insensitive string searching via `strcasestr` is slow as it converts characters dynamically per comparison. Normalizing haystacks to lowercase once per scope allows exact matching via `strstr`, which is significantly faster and simplifies logic. Additionally, substring matching like searching for "ai" can trigger false positives on words like "blockchain" containing "ai", which can be solved with precise non-alphanumeric/underscore word boundaries (`has_word`).
**Action:** Always pre-normalize user input strings to lowercase once per scope to replace multiple expensive `strcasestr` calls with direct `strstr` calls, and employ boundary checks for short keywords to prevent false positives.
