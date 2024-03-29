---
Title: Constants
---

#- Constants
- `SM_INT64_SUPPORTED` = `SOURCEMOD_V_MAJOR >= 1 && SOURCEMOD_V_MINOR >= 11 && SOURCEMOD_V_REV >= 6861`
- 
- `JSON_NONE` = `0`
- Used when no options are desired.
- `JSON_ENCODE_PRETTY` = `1 << 0`
- Should encoded output be pretty printed?
- `JSON_DECODE_SINGLE_QUOTES` = `1 << 0`
- Should single quote wrapped strings be accepted during decoding?
- `JSON_MERGE_REPLACE` = `1 << 0`
- During merge, should existing keys be replaced if they exist in both objects?
- `JSON_MERGE_CLEANUP` = `1 << 1`
- During merge, should existing objects be cleaned up if they exist in both objects? (only applies when JSON_MERGE_REPLACE is also set)
- `JSON_INT_BUFFER_SIZE` = `12`
- The longest representable integer ("-2147483648") + NULL terminator
- `JSON_INT64_BUFFER_SIZE` = `21`
- The longest representable int64 ("-9223372036854775808") + NULL terminator
- `JSON_FLOAT_BUFFER_SIZE` = `32`
- You may need to change this if you are working with large floats.
- `JSON_BOOL_BUFFER_SIZE` = `6`
- "true"|"false" + NULL terminator
- `JSON_NULL_BUFFER_SIZE` = `5`
- "null" + NULL terminator
- `JSON_ARRAY_KEY` = `"is_array"`
- 
- `JSON_ENFORCE_TYPE_KEY` = `"enforce_type"`
- 
- `TRIE_SUPPORTS_CONTAINSKEY` = `SOURCEMOD_V_MAJOR >= 1 && SOURCEMOD_V_MINOR >= 11 && SOURCEMOD_V_REV >= 6646`
- For more information, see [StringMap.ContainsKey](https://sm.alliedmods.net/new-api/adt_trie/StringMap/ContainsKey).
